#include "MapView.h"
#include <QSGGeometry>
#include <QSGVertexColorMaterial>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cmath>
#include <numeric>
#include <queue>

MapView::MapView(QQuickItem *parent) : QQuickItem(parent) {
    setFlag(ItemHasContents, true);
}

void MapView::setGraph(Graph* graph) {
    if (m_graph != graph) {
        m_graph = graph;
        m_rangeDirty = true;
        m_degreeDirty = true;
        emit graphChanged();
        update();
    }
}

void MapView::refresh() {
    m_rangeDirty = true;
    m_degreeDirty = true;
    update();
}

QVariantMap MapView::hitTestNode(const QPointF& screenPos, double tolerance) const {
    QVariantMap result;
    if (!m_graph) return result;
    
    double bestDist = tolerance;
    const Node* bestNode = nullptr;

    const auto& nodes = m_graph->getAllNodes();
    for (const auto& node : nodes) {
        QPointF spos = mapToScreen(node.x, node.y);
        double dist = std::hypot(spos.x() - screenPos.x(), spos.y() - screenPos.y());
        if (dist < bestDist) {
            bestDist = dist;
            bestNode = &node;
        }
    }

    if (bestNode) {
        result["found"] = true;
        result["id"] = bestNode->Node_id;
        result["x"] = bestNode->x;
        result["y"] = bestNode->y;
    } else {
        result["found"] = false;
    }
    return result;
}

QVariantMap MapView::hitTestEdge(const QPointF& screenPos, double tolerance) const {
    QVariantMap result;
    if (!m_graph) return result;
    
    double bestDist = tolerance;
    const Edge* bestEdge = nullptr;

    const auto& edges = m_graph->getAllEdges();
    for (const auto& edge : edges) {
        Node n1 = m_graph->getNode(edge.source);
        Node n2 = m_graph->getNode(edge.target);
        
        QPointF p1 = mapToScreen(n1.x, n1.y);
        QPointF p2 = mapToScreen(n2.x, n2.y);
        
        double l2 = std::pow(p2.x() - p1.x(), 2) + std::pow(p2.y() - p1.y(), 2);
        double dist;
        if (l2 == 0.0) {
            dist = std::hypot(screenPos.x() - p1.x(), screenPos.y() - p1.y());
        } else {
            double t = std::clamp(((screenPos.x() - p1.x()) * (p2.x() - p1.x()) + (screenPos.y() - p1.y()) * (p2.y() - p1.y())) / l2, 0.0, 1.0);
            QPointF proj(p1.x() + t * (p2.x() - p1.x()), p1.y() + t * (p2.y() - p1.y()));
            dist = std::hypot(screenPos.x() - proj.x(), screenPos.y() - proj.y());
        }

        if (dist < bestDist) {
            bestDist = dist;
            bestEdge = &edge;
        }
    }

    if (bestEdge) {
        result["found"] = true;
        result["source"] = bestEdge->source;
        result["target"] = bestEdge->target;
        result["capacity"] = bestEdge->capacity;
        result["length"] = bestEdge->length;
        result["currentCars"] = bestEdge->currentCars;
    } else {
        result["found"] = false;
    }
    return result;
}

void MapView::setZoom(double z) {
    if (m_zoom != z) {
        m_zoom = z;
        emit zoomChanged();
        update();
    }
}

void MapView::setOffset(const QPointF& o) {
    if (m_offset != o) {
        m_offset = o;
        emit offsetChanged();
        update();
    }
}

void MapView::updateRange() {
    if (!m_graph || !m_rangeDirty) return;
    const auto& nodes = m_graph->getAllNodes();
    if (nodes.empty()) return;

    m_range.minX = m_range.maxX = nodes[0].x;
    m_range.minY = m_range.maxY = nodes[0].y;
    for (const auto& node : nodes) {
        if (node.x < m_range.minX) m_range.minX = node.x;
        if (node.x > m_range.maxX) m_range.maxX = node.x;
        if (node.y < m_range.minY) m_range.minY = node.y;
        if (node.y > m_range.maxY) m_range.maxY = node.y;
    }
    m_rangeDirty = false;
}

void MapView::updateDegreeOrder() {
    if (!m_graph || !m_degreeDirty) return;
    const auto& nodes = m_graph->getAllNodes();
    const auto& edges = m_graph->getAllEdges();
    int n = static_cast<int>(nodes.size());
    if (n == 0) return;

    // 统计每个节点的总度数（入度 + 出度）
    std::unordered_map<int, int> deg;
    deg.reserve(n);
    for (const auto& nd : nodes) deg[nd.Node_id] = 0;
    for (const auto& e  : edges) { deg[e.source]++; deg[e.target]++; }

    // 按度数降序排列 nodes[] 的下标
    m_nodesByDegree.resize(n);
    std::iota(m_nodesByDegree.begin(), m_nodesByDegree.end(), 0);
    std::sort(m_nodesByDegree.begin(), m_nodesByDegree.end(),
              [&](int a, int b){ return deg[nodes[a].Node_id] > deg[nodes[b].Node_id]; });

    m_degreeDirty = false;
}

QPointF MapView::mapToScreen(double x, double y) const {
    double dx = m_range.maxX - m_range.minX;
    double dy = m_range.maxY - m_range.minY;
    if (dx <= 0) dx = 1;
    if (dy <= 0) dy = 1;

    double baseScale = std::min(width() / dx, height() / dy);
    double startX = (width()  - dx * baseScale) / 2.0;
    double startY = (height() - dy * baseScale) / 2.0;

    double sx = startX + (x - m_range.minX) * baseScale * m_zoom + m_offset.x() * width();
    double sy = startY + (y - m_range.minY) * baseScale * m_zoom + m_offset.y() * height();
    return QPointF(sx, sy);
}

QSGNode *MapView::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) {
    if (!m_graph) return oldNode;
    updateRange();
    updateDegreeOrder();

    QSGNode *root = oldNode;
    if (!root) root = new QSGNode();

    const auto& edges = m_graph->getAllEdges();
    const auto& nodes = m_graph->getAllNodes();
    int totalNodeCount = static_cast<int>(nodes.size());

    // ── 0. 度数 LOD：决定渲染前 N 个最高度数节点 ────────────────────────────
    double worldW = m_range.maxX - m_range.minX;
    double worldH = m_range.maxY - m_range.minY;
    if (worldW <= 0) worldW = 1;
    if (worldH <= 0) worldH = 1;

    double baseScale     = std::min(width() / worldW, height() / worldH);
    double pixelsPerUnit = baseScale * m_zoom;

    // ── LOD 开关 ──────────────────────────────────────────────────────────────
    int visibleCount = totalNodeCount; // 默认全量

    if (m_lodEnabled) {
        const int MIN_VISIBLE = 200;
        int targetVisible = static_cast<int>(
            std::clamp(MIN_VISIBLE * std::pow(pixelsPerUnit / baseScale, 1.5)
                       * static_cast<double>(totalNodeCount) / 300.0,
                       static_cast<double>(MIN_VISIBLE),
                       static_cast<double>(totalNodeCount)));
        visibleCount = std::min(targetVisible, static_cast<int>(m_nodesByDegree.size()));
    }

    // visibleSet: Node_id -> nodes[] 下标
    std::unordered_map<int, int> visibleSet;
    visibleSet.reserve(visibleCount);
    for (int k = 0; k < visibleCount; ++k) {
        int idx = m_nodesByDegree[k];
        visibleSet[nodes[idx].Node_id] = idx;
    }

    // ── 1. 多源 BFS（仅 LOD 开启时需要虚拟边；关闭时直接用原始边）─────────────
    //
    // 从所有可见节点同时向外 BFS，经过隐藏节点传播。
    // nodeOwner[id] = 最近可见节点的 Node_id（-1 = 未访问）
    //
    // 之后扫描所有原始边：若边的两端拥有不同领主，则在两个领主之间补绘虚拟边。
    // 这样无论两个高度数节点之间经过多少个隐藏节点，都能正确连线。
    std::unordered_map<int, int> nodeOwner;   // Node_id -> owner visible Node_id
    nodeOwner.reserve(totalNodeCount);

    std::queue<int> bfsQ;

    // 初始化：可见节点自己是自己的领主
    for (const auto& [vid, vIdx] : visibleSet) {
        nodeOwner[vid] = vid;
        bfsQ.push(vid);
    }

    // BFS 扩散：让可见节点"占领"相邻的隐藏节点
    while (!bfsQ.empty()) {
        int cur = bfsQ.front(); bfsQ.pop();
        int owner = nodeOwner[cur];
        for (const auto& e : m_graph->getEdgesFrom(cur)) {
            if (!nodeOwner.count(e.target)) {
                nodeOwner[e.target] = owner;
                bfsQ.push(e.target);
            }
        }
    }

    // ── 2. 扫描所有边，收集虚拟边 ────────────────────────────────────────────
    //
    // 对每条原始边 (u, v)：
    //   - 找到 u 的领主 ownerU，v 的领主 ownerV
    //   - 若 ownerU != ownerV，在它们之间添加虚拟边
    // 用 map<min,map<max,ratio>> 去重，保证每对节点只画一条线。
    struct EdgeData { double ratio; int capacity; };
    std::unordered_map<int, std::unordered_map<int, EdgeData>> vmap;

    auto addVirtual = [&](int a, int b, double ratio, int capacity) {
        if (a == b) return;
        int lo = std::min(a, b), hi = std::max(a, b);
        if (!vmap[lo].count(hi)) {
            vmap[lo][hi] = {ratio, capacity};
        } else {
            vmap[lo][hi].capacity = std::max(vmap[lo][hi].capacity, capacity);
            vmap[lo][hi].ratio = std::max(vmap[lo][hi].ratio, ratio);
        }
    };

    for (const auto& edge : edges) {
        auto itSrc = nodeOwner.find(edge.source);
        auto itDst = nodeOwner.find(edge.target);
        if (itSrc == nodeOwner.end() || itDst == nodeOwner.end()) continue;

        int ownerSrc = itSrc->second;
        int ownerDst = itDst->second;
        if (ownerSrc == ownerDst) continue;  // 同一领主，不需要跨边

        double ratio = (edge.capacity > 0)
                       ? static_cast<double>(edge.currentCars) / edge.capacity : 0;
        addVirtual(ownerSrc, ownerDst, ratio, edge.capacity);
    }

    // ── 3. 清理旧节点 ─────────────────────────────────────────────────────────
    root->removeAllChildNodes();

    // ── 4. 构建边的渲染节点（ Chunking ）───────────────────────────────────────
    // 为了防止顶点数超过 16-bit 渲染限制 (65535)，每批最多 10000 条边 (60000 顶点)
    const int MAX_ITEMS_PER_CHUNK = 10000;
    
    // 找出当前 hover 的两端节点，在 LOD 下映射为显示网格中的宏节点
    int hoverLo = -1, hoverHi = -1;
    if (m_hoveredEdgeSource != -1 && m_hoveredEdgeTarget != -1) {
        int u = nodeOwner.count(m_hoveredEdgeSource) ? nodeOwner[m_hoveredEdgeSource] : m_hoveredEdgeSource;
        int v = nodeOwner.count(m_hoveredEdgeTarget) ? nodeOwner[m_hoveredEdgeTarget] : m_hoveredEdgeTarget;
        hoverLo = std::min(u, v);
        hoverHi = std::max(u, v);
    }

    // 收集所有需要绘制的边数据
    struct DrawEdge {
        QPointF p1, p2;
        double ratio;
        int capacity;
        bool isHovered;
    };
    std::vector<DrawEdge> drawEdges;
    for (const auto& outerPair : vmap) {
        int lo = outerPair.first;
        Node n1 = m_graph->getNode(lo);
        QPointF p1 = mapToScreen(n1.x, n1.y);
        for (const auto& innerPair : outerPair.second) {
            int hi = innerPair.first;
            Node n2 = m_graph->getNode(hi);
            QPointF p2 = mapToScreen(n2.x, n2.y);
            
            bool isHovered = (lo == hoverLo && hi == hoverHi);
            drawEdges.push_back({p1, p2, innerPair.second.ratio, innerPair.second.capacity, isHovered});
        }
    }

    int ve = static_cast<int>(drawEdges.size());
    for (int chunkStart = 0; chunkStart < ve; chunkStart += MAX_ITEMS_PER_CHUNK) {
        int chunkEnd = std::min(ve, chunkStart + MAX_ITEMS_PER_CHUNK);
        int chunkSize = chunkEnd - chunkStart;

        QSGGeometryNode *edgeNode = new QSGGeometryNode();
        edgeNode->setMaterial(new QSGVertexColorMaterial());
        edgeNode->setFlag(QSGNode::OwnsMaterial);
        edgeNode->setFlag(QSGNode::OwnsGeometry);

        QSGGeometry *edgeGeom = new QSGGeometry(
            QSGGeometry::defaultAttributes_ColoredPoint2D(), chunkSize * 6);
        edgeGeom->setDrawingMode(QSGGeometry::DrawTriangles);
        auto *eVerts = edgeGeom->vertexDataAsColoredPoint2D();

        for (int i = 0; i < chunkSize; ++i) {
            const auto& edge = drawEdges[chunkStart + i];
            double dx = edge.p2.x() - edge.p1.x();
            double dy = edge.p2.y() - edge.p1.y();
            double len = std::hypot(dx, dy);

            int base = i * 6;
            if (len < 1e-5) {
                for (int j = 0; j < 6; ++j) eVerts[base + j].set(0, 0, 0, 0, 0, 0);
                continue;
            }
            
            double nx = dy / len;
            double ny = -dx / len;

            // 最低像素宽度设为 2.5
            double width = std::max(2.5, edge.capacity * pixelsPerUnit * 0.05);
            if (edge.isHovered) {
                width *= 2.5; // 加粗
            }

            double wx = nx * width / 2.0;
            double wy = ny * width / 2.0;

            unsigned char r, g, b;
            if (edge.isHovered) {
                r = 255; g = 64; b = 129; // 高亮品红
            } else if (edge.ratio < 0.5) { r = 0;   g = 255; b = 0;   }
            else if (edge.ratio < 0.9) { r = 255; g = 255; b = 0;   }
            else                       { r = 255; g = 0;   b = 0;   }
            unsigned char a = 255;

            eVerts[base + 0].set(edge.p1.x() - wx, edge.p1.y() - wy, r, g, b, a);
            eVerts[base + 1].set(edge.p1.x() + wx, edge.p1.y() + wy, r, g, b, a);
            eVerts[base + 2].set(edge.p2.x() - wx, edge.p2.y() - wy, r, g, b, a);
            eVerts[base + 3].set(edge.p2.x() - wx, edge.p2.y() - wy, r, g, b, a);
            eVerts[base + 4].set(edge.p1.x() + wx, edge.p1.y() + wy, r, g, b, a);
            eVerts[base + 5].set(edge.p2.x() + wx, edge.p2.y() + wy, r, g, b, a);
        }
        edgeNode->setGeometry(edgeGeom);
        root->appendChildNode(edgeNode);
    }

    // ── 5. 构建节点的渲染节点（ Chunking ）────────────────────────────────────
    float halfSize = static_cast<float>(std::clamp(pixelsPerUnit * 0.4, 1.5, 4.0));

    for (int chunkStart = 0; chunkStart < visibleCount; chunkStart += MAX_ITEMS_PER_CHUNK) {
        int chunkEnd = std::min(visibleCount, chunkStart + MAX_ITEMS_PER_CHUNK);
        int chunkSize = chunkEnd - chunkStart;

        QSGGeometryNode *pointNode = new QSGGeometryNode();
        pointNode->setMaterial(new QSGVertexColorMaterial());
        pointNode->setFlag(QSGNode::OwnsMaterial);
        pointNode->setFlag(QSGNode::OwnsGeometry);

        QSGGeometry *pointGeom = new QSGGeometry(
            QSGGeometry::defaultAttributes_ColoredPoint2D(), chunkSize * 6);
        pointGeom->setDrawingMode(QSGGeometry::DrawTriangles);
        auto *pVerts = pointGeom->vertexDataAsColoredPoint2D();

        for (int i = 0; i < chunkSize; ++i) {
            int idx = m_nodesByDegree[chunkStart + i];
            QPointF p  = mapToScreen(nodes[idx].x, nodes[idx].y);
            float px   = static_cast<float>(p.x());
            float py   = static_cast<float>(p.y());
            unsigned char r = 0, g = 150, b = 255, a = 255;

            int base = i * 6;
            pVerts[base + 0].set(px - halfSize, py - halfSize, r, g, b, a);
            pVerts[base + 1].set(px + halfSize, py - halfSize, r, g, b, a);
            pVerts[base + 2].set(px - halfSize, py + halfSize, r, g, b, a);
            pVerts[base + 3].set(px + halfSize, py - halfSize, r, g, b, a);
            pVerts[base + 4].set(px + halfSize, py + halfSize, r, g, b, a);
            pVerts[base + 5].set(px - halfSize, py + halfSize, r, g, b, a);
        }
        pointNode->setGeometry(pointGeom);
        root->appendChildNode(pointNode);
    }

    return root;
}
