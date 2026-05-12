#!/usr/bin/env python3
"""
将 OpenStreetMap 真实道路网转换为项目所需的 map_data.json 格式。
已增强：加入 POI (兴趣点) 地名匹配与进度条提示。

用法:
    python osm_to_json.py "北京市"                    # 默认 drive 路网
    python osm_to_json.py "上海市" --mode walk        # 步行路网
    python osm_to_json.py "南京市" --output nanjing.json
    python osm_to_json.py --list                      # 列出已生成的地图文件

依赖:
    pip install osmnx tqdm
"""

import argparse
import json
import os
import sys
import time
from pathlib import Path

# ── 输出目录：项目根目录下的 map_data/ ────────────────────────────────
PROJECT_ROOT = Path(__file__).resolve().parent.parent
OUTPUT_DIR = PROJECT_ROOT / "map_data"
OUTPUT_DIR.mkdir(exist_ok=True)


def download_city(city_name: str, mode: str = "drive") -> dict:
    """下载城市路网并转换为项目 JSON 格式。"""
    try:
        import osmnx as ox
        from tqdm import tqdm
    except ImportError:
        sys.exit(
            "请先安装 osmnx 和 tqdm:\n"
            "  pip install osmnx tqdm\n"
            "注意: osmnx 依赖较多，建议在虚拟环境中安装。"
        )

    print(f"正在下载 {city_name} 的 {mode} 路网数据 (可能需要几分钟)...")
    t_start = time.time()

    try:
        G = ox.graph_from_place(city_name, network_type=mode, simplify=True)
    except ValueError as e:
        sys.exit(f"未找到匹配 '{city_name}' 的区域。请尝试更具体的名称，如 '北京市, China'。\n原始错误: {e}")
    except Exception as e:
        sys.exit(f"下载失败: {e}")

    t_dl = time.time()
    print(f"  → 路网下载完成，耗时 {t_dl - t_start:.1f}s")
    print(f"  → 原始节点数: {G.number_of_nodes()}, 原始边数: {G.number_of_edges()}")

    # ── 投影到 UTM 坐标系（米），与模拟地图量级一致 ──────────────────────
    print("正在投影到 UTM 坐标系...")
    G = ox.project_graph(G)
    print(f"  → 投影完成")

    # ── 获取周边 POI 并匹配到节点 ──────────────────────────────────────
    print("正在下载并匹配周边兴趣点 (POI)...")
    try:
        # 下载包含 name 标签的设施、建筑、店铺等
        pois = ox.features_from_place(city_name, tags={'name': True, 'amenity': True, 'building': True, 'shop': True})
        
        if not pois.empty:
            print(f"  → 共下载到 {len(pois)} 个兴趣点，正在过滤和匹配...")
            # 过滤掉没有几何信息或没有名字的行
            pois = pois[pois.geometry.notnull() & pois['name'].notnull()]
            
            # 将 POI 投影到与路网图相同的 UTM 坐标系
            pois = pois.to_crs(G.graph['crs'])
            centroids = pois.geometry.centroid
            
            # 找到每个 POI 离得最近的路网节点
            print("  → 计算 POI 与节点的最近距离映射...")
            nearest_nodes = ox.distance.nearest_nodes(G, centroids.x, centroids.y)
            
            # 将 POI 的名字赋予对应的节点
            for poi_name, node_id in zip(pois['name'], nearest_nodes):
                if isinstance(poi_name, str) and poi_name.strip():
                    G.nodes[node_id]['poi_name'] = poi_name.strip()
            print(f"  → 成功提取并映射了 {len(pois)} 个兴趣点！")
        else:
            print("  → 未找到带有名称的兴趣点。")
    except Exception as e:
        print(f"  → 提取 POI 失败，跳过该步骤: {e}")

    # ── 构建节点列表 ───────────────────────────────────────────────────
    print("正在转换节点...")
    nodes = []
    osm_to_new = {}

    for i, osm_id in enumerate(tqdm(G.nodes(), desc="处理节点")):
        node_data = G.nodes[osm_id]
        osm_to_new[osm_id] = i
        
        # 提取节点自身的名字 (如果有的话)
        node_self_name = node_data.get("name", "")
        if isinstance(node_self_name, list):
            node_self_name = node_self_name[0]
            
        # 优先使用匹配到的 POI 名字，其次用自带名字
        final_name = node_data.get('poi_name', node_self_name)

        nodes.append({
            "id": i,
            "x": round(node_data["x"], 2),  # UTM easting (米)
            "y": round(node_data["y"], 2),  # UTM northing (米)
            "name": final_name,
        })

    # ── 从边数据派生节点地名兜底 ───────────────────────────────────────
    highway_priority = {
        "motorway": 12, "motorway_link": 11,
        "trunk": 10, "trunk_link": 9,
        "primary": 8, "primary_link": 7,
        "secondary": 6, "secondary_link": 5,
        "tertiary": 4, "tertiary_link": 3,
        "residential": 2, "living_street": 1,
        "unclassified": 0, "service": -1,
        "footway": -2, "path": -3, "steps": -4,
    }
    node_best_name = {}

    def _normalize_name(val):
        if isinstance(val, str):
            return val
        if isinstance(val, (list, tuple)):
            return str(val[0]) if val else ""
        return ""

    for u, v, key, data in G.edges(keys=True, data=True):
        raw_name = data.get("name")
        if not raw_name or key != 0:
            continue
        name = _normalize_name(raw_name)
        if not name:
            continue
        highway = data.get("highway", "")
        if isinstance(highway, list):
            highway = highway[0] if highway else ""
        priority = highway_priority.get(str(highway), -99)

        for osm_nid in (u, v):
            cur = node_best_name.get(osm_nid)
            if cur is None or priority > cur[0]:
                node_best_name[osm_nid] = (priority, name)

    # 如果节点还没名字，用连接的道路名作为地名
    for osm_id, (_, name) in tqdm(node_best_name.items(), desc="道路名兜底"):
        idx = osm_to_new.get(osm_id)
        if idx is not None:
            if not nodes[idx]["name"]:
                nodes[idx]["name"] = name

    named_count = sum(1 for n in nodes if n["name"])
    print(f"  → 最终 {named_count}/{len(nodes)} 个节点拥有地名")

    # ── 构建边列表 ─────────────────────────────────────────────────────
    print("正在转换边...")
    edges = []
    edge_id = 0

    capacity_map = {
        "motorway":     1200, "motorway_link": 800,
        "trunk":         900, "trunk_link":    600,
        "primary":       700, "primary_link":  500,
        "secondary":     500, "secondary_link":350,
        "tertiary":      350, "tertiary_link": 250,
        "residential":   200, "living_street": 150,
        "unclassified":  200, "service":       120,
        "footway":        20, "path":           20,
        "steps":          10,
    }

    # 为了进度条正确显示，将边转换为 list
    edge_list = list(G.edges(keys=True, data=True))
    for u, v, key, data in tqdm(edge_list, desc="处理边"):
        if key != 0:
            continue

        src = osm_to_new.get(u)
        tgt = osm_to_new.get(v)
        if src is None or tgt is None:
            continue

        length = data.get("length", 0.0)

        lanes = data.get("lanes")
        if lanes is not None and isinstance(lanes, (int, float)) and lanes > 0:
            capacity = float(lanes) * 250.0
        else:
            highway = data.get("highway", "")
            if isinstance(highway, list):
                highway = highway[0] if highway else ""
            capacity = capacity_map.get(str(highway), 150)

        edges.append({
            "id": edge_id,
            "source": src,
            "target": tgt,
            "length": round(length, 2),
            "capacity": round(capacity, 1),
            "currentCars": 0,
        })
        edge_id += 1

    t_conv = time.time()
    print(f"  → 转换完成，总耗时 {t_conv - t_start:.1f}s")
    print(f"  → 输出节点数: {len(nodes)}, 输出边数: {len(edges)}")

    return {"nodes": nodes, "edges": edges}


def main():
    parser = argparse.ArgumentParser(
        description="下载 OSM 真实道路网并转换为项目 JSON 格式"
    )
    parser.add_argument(
        "city", nargs="?", default=None,
        help="城市名称，如 '北京市' 或 '北京市, China'"
    )
    parser.add_argument(
        "--mode", default="drive",
        choices=["drive", "walk", "bike", "all"],
        help="路网类型 (默认 drive)"
    )
    parser.add_argument(
        "--output", "-o", default=None,
        help="输出文件名 (不含路径，默认自动生成)"
    )
    parser.add_argument(
        "--list", "-l", action="store_true",
        help="列出已生成的地图文件"
    )

    args = parser.parse_args()

    if args.list:
        files = sorted(OUTPUT_DIR.glob("*.json"))
        if not files:
            print(f"目录 {OUTPUT_DIR} 中暂无地图文件。")
        else:
            print(f"已生成的地图文件 ({OUTPUT_DIR}):")
            for f in files:
                size_mb = f.stat().st_size / (1024 * 1024)
                print(f"  {f.name}  ({size_mb:.1f} MB)")
        return

    if not args.city:
        parser.print_help()
        sys.exit("\n请指定城市名称。")

    city = args.city
    if "," not in city:
        city = f"{args.city}, China"

    data = download_city(city, args.mode)

    if args.output:
        filename = args.output if args.output.endswith(".json") else args.output + ".json"
    else:
        safe_name = args.city.replace(", ", "_").replace(" ", "_")
        filename = f"{safe_name}_{args.mode}.json"

    output_path = OUTPUT_DIR / filename
    with open(output_path, "w", encoding="utf-8") as f:
        json.dump(data, f, ensure_ascii=False)

    size_mb = output_path.stat().st_size / (1024 * 1024)
    print(f"\n✓ 已保存到: {output_path} ({size_mb:.1f} MB)")

    print(f"\n使用方法:")
    print(f"  1. 将程序中的 Graph::load() 路径改为该文件")
    print(f"  2. 或复制到项目目录: cp {output_path} ../City-Navigation-System/map_data.json")


if __name__ == "__main__":
    main()
