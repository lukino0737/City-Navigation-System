#include "MapView.h"
#include <QSGGeometry>
#include <QSGVertexColorMaterial>
#include <QColor>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cmath>
#include <numeric>

MapView::MapView(QQuickItem *parent) : QQuickItem(parent) {
    setFlag(ItemHasContents, true);
}

void MapView::setGraph(Graph* graph) {
    if (m_graph != graph) {
        m_graph = graph;
        m_rangeDirty = true;
        m_degreeDirty = true; // 图换了，度数顺序也需要重建
        emit graphChanged();
        update();
    }
}

void MapView::refresh() {
    m_rangeDirty = true;
    m_degreeDirty = true;
    update();
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

// 预计算：统计每个节点的度数，然后按度数降序排列节点下标
void MapView::updateDegreeOrder() {
    if (!m_graph || !m_degreeDirty) return;

    const auto& nodes = m_graph->getAllNodes();
    const auto& edges = m_graph->getAllEdges();
    int n = static_cast<int>(nodes.size());
    if (n == 0) return;

    // 统计每个 Node_id 对应节点的出度（连边数）
    std::unordered_map<int, int> degreeMap;
    degreeMap.reserve(n);
    for (const auto& node : nodes) degreeMap[node.Node_id] = 0;
    for (const auto& edge : edges) {
        degreeMap[edge.source]++;
        degreeMap[edge.target]++;
    }

    // 建立 节点下标 的列表并按度数排序
    m_nodesByDegree.resize(n);
    std::iota(m_nodesByDegree.begin(), m_nodesByDegree.end(), 0); // 0, 1, 2, ...

    std::sort(m_nodesByDegree.begin(), m_nodesByDegree.end(),
              [&](int a, int b) {
                  return degreeMap[nodes[a].Node_id] > degreeMap[nodes[b].Node_id];
              });

    m_degreeDirty = false;
}

QPointF MapView::mapToScreen(double x, double y) const {
    double dx = m_range.maxX - m_range.minX;
    double dy = m_range.maxY - m_range.minY;
    if (dx <= 0) dx = 1;
    if (dy <= 0) dy = 1;

    // 保持长宽比：取 X / Y 两方向较小的缩放比作为基础比例
    double scaleX = width() / dx;
    double scaleY = height() / dy;
    double baseScale = std::min(scaleX, scaleY);

    // 居中留白边距（宽屏时左右留白，竖屏时上下留白）
    double startX = (width()  - dx * baseScale) / 2.0;
    double startY = (height() - dy * baseScale) / 2.0;

    // 应用居中、基础缩放、鼠标滚轮缩放 m_zoom 及拖拽偏移 m_offset
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

    // ── 0. 度数 LOD：根据当前缩放级别确定需要显示的节点数量 ─────────────────
    //
    // 计算 "每个世界单位对应多少屏幕像素"，综合了基础缩放和用户滚轮缩放
    double worldW = m_range.maxX - m_range.minX;
    double worldH = m_range.maxY - m_range.minY;
    if (worldW <= 0) worldW = 1;
    if (worldH <= 0) worldH = 1;
    double baseScale     = std::min(width() / worldW, height() / worldH);
    double pixelsPerUnit = baseScale * m_zoom;

    // 根据缩放比动态决定最多显示多少个节点：
    //   缩放比越大（越放大），显示越多；最少显示 200 个（保留骨架），最多全量显示
    //   这里用 pixelsPerUnit^1.5 进行平滑映射，可按需调参
    const int MIN_VISIBLE = 200;
    int targetVisible = static_cast<int>(
        std::clamp(MIN_VISIBLE * std::pow(pixelsPerUnit / baseScale, 1.5)
                   * static_cast<double>(totalNodeCount) / 300.0,
                   static_cast<double>(MIN_VISIBLE),
                   static_cast<double>(totalNodeCount)));

    // 取 m_nodesByDegree 前 targetVisible 个节点（度数最高的那些）
    int visibleCount = std::min(targetVisible, static_cast<int>(m_nodesByDegree.size()));

    // 建立可见节点集合：key = Node_id，value = nodes[] 下标，用于快速查找
    std::unordered_map<int, int> visibleSet;
    visibleSet.reserve(visibleCount);
    for (int k = 0; k < visibleCount; ++k) {
        int nodeIdx = m_nodesByDegree[k];
        visibleSet[nodes[nodeIdx].Node_id] = nodeIdx;
    }

    // ── 1. 渲染可见的边（两端点均在 visibleSet 里的边才绘制）─────────────────
    QSGGeometryNode *edgeNode = nullptr;
    bool isNewEdge = false;
    if (root->childCount() > 0) {
        edgeNode = static_cast<QSGGeometryNode *>(root->childAtIndex(0));
    } else {
        edgeNode = new QSGGeometryNode();
        edgeNode->setMaterial(new QSGVertexColorMaterial());
        edgeNode->setFlag(QSGNode::OwnsMaterial);
        edgeNode->setFlag(QSGNode::OwnsGeometry);
        isNewEdge = true;
    }

    std::vector<int> visibleEdgeIdx;
    visibleEdgeIdx.reserve(visibleCount * 4);
    for (int i = 0; i < static_cast<int>(edges.size()); ++i) {
        if (visibleSet.count(edges[i].source) && visibleSet.count(edges[i].target))
            visibleEdgeIdx.push_back(i);
    }

    int ve = static_cast<int>(visibleEdgeIdx.size());
    QSGGeometry *edgeGeom = new QSGGeometry(
        QSGGeometry::defaultAttributes_ColoredPoint2D(), ve * 2);
    edgeGeom->setDrawingMode(QSGGeometry::DrawLines);
    edgeGeom->setLineWidth(1.0f);
    auto *eVerts = edgeGeom->vertexDataAsColoredPoint2D();

    for (int i = 0; i < ve; ++i) {
        const auto& edge = edges[visibleEdgeIdx[i]];
        Node n1 = m_graph->getNode(edge.source);
        Node n2 = m_graph->getNode(edge.target);
        QPointF p1 = mapToScreen(n1.x, n1.y);
        QPointF p2 = mapToScreen(n2.x, n2.y);

        double ratio = (edge.capacity > 0)
                       ? static_cast<double>(edge.currentCars) / edge.capacity : 0;
        unsigned char r, g, b;
        if      (ratio < 0.5) { r = 0;   g = 255; b = 0;   }
        else if (ratio < 0.9) { r = 255; g = 255; b = 0;   }
        else                  { r = 255; g = 0;   b = 0;   }

        eVerts[i * 2    ].set(p1.x(), p1.y(), r, g, b, 255);
        eVerts[i * 2 + 1].set(p2.x(), p2.y(), r, g, b, 255);
    }
    edgeNode->setGeometry(edgeGeom);
    edgeNode->markDirty(QSGNode::DirtyGeometry);
    if (isNewEdge) root->appendChildNode(edgeNode);

    // ── 2. 渲染可见的节点（矩形 = 2 三角形 × 6 顶点）────────────────────────
    QSGGeometryNode *pointNode = nullptr;
    bool isNewPoint = false;
    if (root->childCount() > 1) {
        pointNode = static_cast<QSGGeometryNode *>(root->childAtIndex(1));
    } else {
        pointNode = new QSGGeometryNode();
        pointNode->setMaterial(new QSGVertexColorMaterial());
        pointNode->setFlag(QSGNode::OwnsMaterial);
        pointNode->setFlag(QSGNode::OwnsGeometry);
        isNewPoint = true;
    }

    QSGGeometry *pointGeom = new QSGGeometry(
        QSGGeometry::defaultAttributes_ColoredPoint2D(), visibleCount * 6);
    pointGeom->setDrawingMode(QSGGeometry::DrawTriangles);
    auto *pVerts = pointGeom->vertexDataAsColoredPoint2D();

    // 缩放越大，节点方块也略大（最小 1.5px，最大 4px）
    float halfSize = static_cast<float>(std::clamp(pixelsPerUnit * 0.4, 1.5, 4.0));

    for (int k = 0; k < visibleCount; ++k) {
        int nodeIdx = m_nodesByDegree[k];
        QPointF p   = mapToScreen(nodes[nodeIdx].x, nodes[nodeIdx].y);
        float px    = static_cast<float>(p.x());
        float py    = static_cast<float>(p.y());
        unsigned char r = 0, g = 150, b = 255, a = 255;

        int base = k * 6;
        pVerts[base + 0].set(px - halfSize, py - halfSize, r, g, b, a);
        pVerts[base + 1].set(px + halfSize, py - halfSize, r, g, b, a);
        pVerts[base + 2].set(px - halfSize, py + halfSize, r, g, b, a);
        pVerts[base + 3].set(px + halfSize, py - halfSize, r, g, b, a);
        pVerts[base + 4].set(px + halfSize, py + halfSize, r, g, b, a);
        pVerts[base + 5].set(px - halfSize, py + halfSize, r, g, b, a);
    }
    pointNode->setGeometry(pointGeom);
    pointNode->markDirty(QSGNode::DirtyGeometry);
    if (isNewPoint) root->appendChildNode(pointNode);

    return root;
}
