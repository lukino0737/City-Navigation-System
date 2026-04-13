#include "MapView.h"
#include <QSGGeometry>
#include <QSGVertexColorMaterial>
#include <QColor>

MapView::MapView(QQuickItem *parent) : QQuickItem(parent) {
    setFlag(ItemHasContents, true);
}

void MapView::setGraph(Graph* graph) {
    if (m_graph != graph) {
        m_graph = graph;
        m_rangeDirty = true;
        emit graphChanged();
        update();
    }
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

QPointF MapView::mapToScreen(double x, double y) const {
    double dx = m_range.maxX - m_range.minX;
    double dy = m_range.maxY - m_range.minY;
    if (dx <= 0) dx = 1;
    if (dy <= 0) dy = 1;

    // 归一化到 [0, 1]
    double nx = (x - m_range.minX) / dx;
    double ny = (y - m_range.minY) / dy;

    // 应用缩放和偏移，并映射到 QML 宽度和高度
    double sx = (nx * m_zoom + m_offset.x()) * width();
    double sy = (ny * m_zoom + m_offset.y()) * height();

    return QPointF(sx, sy);
}

QSGNode *MapView::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) {
    if (!m_graph) return oldNode;
    updateRange();

    QSGNode *root = oldNode;
    if (!root) {
        root = new QSGNode();
    }

    const auto& edges = m_graph->getAllEdges();
    const auto& nodes = m_graph->getAllNodes();

    // --- 1. 渲染所有的边 (Edges) ---
    QSGGeometryNode *edgeNode = nullptr;
    bool isNewEdgeNode = false;
    if (root->childCount() > 0) {
        edgeNode = static_cast<QSGGeometryNode *>(root->childAtIndex(0));
    } else {
        edgeNode = new QSGGeometryNode();
        edgeNode->setMaterial(new QSGVertexColorMaterial());
        edgeNode->setFlag(QSGNode::OwnsMaterial);
        edgeNode->setFlag(QSGNode::OwnsGeometry);
        isNewEdgeNode = true;
    }

    int edgeCount = static_cast<int>(edges.size());
    QSGGeometry *edgeGeom = new QSGGeometry(QSGGeometry::defaultAttributes_ColoredPoint2D(), edgeCount * 2);
    edgeGeom->setDrawingMode(QSGGeometry::DrawLines);
    edgeGeom->setLineWidth(1.0f);
    QSGGeometry::ColoredPoint2D *edgeVertices = edgeGeom->vertexDataAsColoredPoint2D();

    for (int i = 0; i < edgeCount; ++i) {
        const auto& edge = edges[i];
        Node n1 = m_graph->getNode(edge.source);
        Node n2 = m_graph->getNode(edge.target);

        QPointF p1 = mapToScreen(n1.x, n1.y);
        QPointF p2 = mapToScreen(n2.x, n2.y);

        double ratio = (edge.capacity > 0) ? (double)edge.currentCars / (double)edge.capacity : 0;
        unsigned char r, g, b;
        if (ratio < 0.5) { r = 0; g = 255; b = 0; }
        else if (ratio < 0.9) { r = 255; g = 255; b = 0; }
        else { r = 255; g = 0; b = 0; }

        edgeVertices[i * 2].set(p1.x(), p1.y(), r, g, b, 255);
        edgeVertices[i * 2 + 1].set(p2.x(), p2.y(), r, g, b, 255);
    }
    edgeNode->setGeometry(edgeGeom);
    edgeNode->markDirty(QSGNode::DirtyGeometry);
    
    if (isNewEdgeNode) {
        root->appendChildNode(edgeNode);
    }

    // --- 2. 渲染所有的点 (Nodes) ---
    QSGGeometryNode *pointNode = nullptr;
    bool isNewPointNode = false;
    if (root->childCount() > 1) {
        pointNode = static_cast<QSGGeometryNode *>(root->childAtIndex(1));
    } else {
        pointNode = new QSGGeometryNode();
        pointNode->setMaterial(new QSGVertexColorMaterial());
        pointNode->setFlag(QSGNode::OwnsMaterial);
        pointNode->setFlag(QSGNode::OwnsGeometry);
        isNewPointNode = true;
    }

    int nodeCount = static_cast<int>(nodes.size());
    QSGGeometry *pointGeom = new QSGGeometry(QSGGeometry::defaultAttributes_ColoredPoint2D(), nodeCount);
    pointGeom->setDrawingMode(QSGGeometry::DrawPoints);
    QSGGeometry::ColoredPoint2D *pointVertices = pointGeom->vertexDataAsColoredPoint2D();

    for (int i = 0; i < nodeCount; ++i) {
        QPointF p = mapToScreen(nodes[i].x, nodes[i].y);
        pointVertices[i].set(p.x(), p.y(), 0, 150, 255, 255);
    }
    pointNode->setGeometry(pointGeom);
    pointNode->markDirty(QSGNode::DirtyGeometry);

    if (isNewPointNode) {
        root->appendChildNode(pointNode);
    }

    return root;
}
