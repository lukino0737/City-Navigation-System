#!/usr/bin/env python3
"""
将 OpenStreetMap 真实道路网转换为项目所需的 map_data.json 格式。

用法:
    python osm_to_json.py "北京市"                    # 默认 drive 路网
    python osm_to_json.py "上海市" --mode walk        # 步行路网
    python osm_to_json.py "南京市" --output nanjing.json
    python osm_to_json.py --list                      # 列出已生成的地图文件

依赖:
    pip install osmnx
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
    except ImportError:
        sys.exit(
            "请先安装 osmnx:\n"
            "  pip install osmnx\n"
            "注意: osmnx 依赖较多，建议在虚拟环境中安装。"
        )

    print(f"正在下载 {city_name} 的 {mode} 路网数据...")
    t_start = time.time()

    try:
        G = ox.graph_from_place(city_name, network_type=mode, simplify=True)
    except ValueError as e:
        sys.exit(f"未找到匹配 '{city_name}' 的区域。请尝试更具体的名称，如 '北京市, China'。\n原始错误: {e}")
    except Exception as e:
        sys.exit(f"下载失败: {e}")

    t_dl = time.time()
    print(f"  → 下载完成，耗时 {t_dl - t_start:.1f}s")
    print(f"  → 原始节点数: {G.number_of_nodes()}, 原始边数: {G.number_of_edges()}")

    # ── 投影到 UTM 坐标系（米），与模拟地图量级一致 ──────────────────────
    print("正在投影到 UTM 坐标系...")
    G = ox.project_graph(G)
    print(f"  → 投影完成")

    # ── 构建节点列表 ───────────────────────────────────────────────────
    print("正在转换节点...")
    nodes = []
    osm_to_new = {}

    for i, (osm_id) in enumerate(G.nodes()):
        node_data = G.nodes[osm_id]
        osm_to_new[osm_id] = i
        nodes.append({
            "id": i,
            "x": round(node_data["x"], 2),  # UTM easting (米)
            "y": round(node_data["y"], 2),  # UTM northing (米)
        })

    # ── 构建边列表 ─────────────────────────────────────────────────────
    print("正在转换边...")
    edges = []
    edge_id = 0

    # 道路等级 → 默认容量映射
    capacity_map = {
        "motorway":     1200,
        "motorway_link": 800,
        "trunk":         900,
        "trunk_link":    600,
        "primary":       700,
        "primary_link":  500,
        "secondary":     500,
        "secondary_link":350,
        "tertiary":      350,
        "tertiary_link": 250,
        "residential":   200,
        "living_street": 150,
        "unclassified":  200,
        "service":       120,
        "footway":        20,
        "path":           20,
        "steps":          10,
    }

    for u, v, key, data in G.edges(keys=True, data=True):
        # 只保留主要边（多边情况取第一条）
        if key != 0:
            continue

        src = osm_to_new.get(u)
        tgt = osm_to_new.get(v)
        if src is None or tgt is None:
            continue

        # 长度：OSMnx 已在投影坐标系下计算，单位米
        length = data.get("length", 0.0)

        # 容量：优先用车道数推算，否则按道路等级
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
    print(f"  → 转换完成，耗时 {t_conv - t_dl:.1f}s")
    print(f"  → 最终节点数: {len(nodes)}, 最终边数: {len(edges)}")

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

    # ── 列出已有文件 ───────────────────────────────────────────────────
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

    # ── 下载并转换 ─────────────────────────────────────────────────────
    if not args.city:
        parser.print_help()
        sys.exit("\n请指定城市名称。")

    city = args.city
    if "," not in city:
        city = f"{args.city}, China"

    data = download_city(city, args.mode)

    # ── 保存 ───────────────────────────────────────────────────────────
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

    # ── 提示 ───────────────────────────────────────────────────────────
    print(f"\n使用方法:")
    print(f"  1. 将程序中的 Graph::load() 路径改为该文件")
    print(f"  2. 或复制到项目目录: cp {output_path} ../City-Navigation-System/map_data.json")


if __name__ == "__main__":
    main()
