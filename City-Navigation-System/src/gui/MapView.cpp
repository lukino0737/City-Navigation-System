#include "MapView.h"
#include <QSGGeometry>
#include <QSGVertexColorMaterial>
#include <QSGTextureMaterial>
#include <QSGMaterial>
#include <QQuickWindow>
#include <QImage>
#include <QPainter>
#include <QRadialGradient>
#include <QDateTime>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cmath>
#include <numeric>
#include <queue>

MapView::MapView(QQuickItem *parent) : QQuickItem(parent) {
    setFlag(ItemHasContents, true);
    m_zoom = 0.9;
    m_momentumTimer = new QTimer(this);
    m_momentumTimer->setTimerType(Qt::PreciseTimer);
    m_momentumTimer->setInterval(16);  // ~60fps, 原 50ms = 20fps 导致视觉跳帧
    connect(m_momentumTimer, &QTimer::timeout, this, &MapView::onMomentumTick);

    m_animTimer = new QTimer(this);
    m_animTimer->setTimerType(Qt::PreciseTimer);
    m_animTimer->setInterval(16);
    connect(m_animTimer, &QTimer::timeout, this, &MapView::onAnimTick);
}

MapView::~MapView() {
    delete m_nodeTexture;
}

void MapView::setGraph(Graph* graph) {
    if (m_graph != graph) {
        m_graph = graph;
        m_rangeDirty = true;
        m_degreeDirty = true;
        m_topologyDirty = true;
        m_geometryDirty = true;
        emit graphChanged();
        update();
    }
}

void MapView::refresh() {
    m_rangeDirty = true;
    m_degreeDirty = true;
    m_topologyDirty = true;
    m_geometryDirty = true;
    update();
}

QVariantMap MapView::hitTestNode(const QPointF& screenPos, double tolerance) const {
    QVariantMap result;
    if (!m_graph) return result;
    
    QPointF worldPos = mapFromScreen(screenPos.x(), screenPos.y());
    QPointF toleranceOffset = mapFromScreen(screenPos.x() + tolerance, screenPos.y() + tolerance);
    double worldTolX = std::abs(toleranceOffset.x() - worldPos.x());
    double worldTolY = std::abs(toleranceOffset.y() - worldPos.y());
    double worldTol = std::max(worldTolX, worldTolY);

    double bestDist = tolerance * 2; // initial screen distance
    const Node* bestNode = nullptr;

    const auto& nodes = m_graph->getAllNodes();
    for (const auto& node : nodes) {
        if (std::abs(node.x - worldPos.x()) > worldTol || std::abs(node.y - worldPos.y()) > worldTol) {
            continue;
        }

        QPointF spos = mapToScreen(node.x, node.y);
        double dist = std::hypot(spos.x() - screenPos.x(), spos.y() - screenPos.y());
        if (dist < bestDist && dist <= tolerance) {
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
    
    QPointF worldPos = mapFromScreen(screenPos.x(), screenPos.y());
    QPointF toleranceOffset = mapFromScreen(screenPos.x() + tolerance, screenPos.y() + tolerance);
    double worldTolX = std::abs(toleranceOffset.x() - worldPos.x());
    double worldTolY = std::abs(toleranceOffset.y() - worldPos.y());
    double worldTol = std::max(worldTolX, worldTolY);
    
    double minX = worldPos.x() - worldTol;
    double maxX = worldPos.x() + worldTol;
    double minY = worldPos.y() - worldTol;
    double maxY = worldPos.y() + worldTol;

    double bestDist = tolerance * 2;
    const Edge* bestEdge = nullptr;

    const auto& edges = m_graph->getAllEdges();
    for (const auto& edge : edges) {
        Node n1 = m_graph->getNode(edge.source);
        Node n2 = m_graph->getNode(edge.target);
        
        // Fast bounding box rejection
        if (std::min(n1.x, n2.x) > maxX || std::max(n1.x, n2.x) < minX ||
            std::min(n1.y, n2.y) > maxY || std::max(n1.y, n2.y) < minY) {
            continue;
        }

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

        if (dist < bestDist && dist <= tolerance) {
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

QVariantMap MapView::selectNode(int nodeId) {
    QVariantMap info;
    if (!m_graph) return info;

    m_selectedNodeId = nodeId;
    m_selectedEdgeSource = -1;
    m_selectedEdgeTarget = -1;
    m_highlightedNodeIds.clear();
    m_selectionDirty = true;
    m_geometryDirty = true;

    Node n = m_graph->getNode(nodeId);
    info["type"] = "node";
    info["id"] = n.Node_id;
    info["x"] = n.x;
    info["y"] = n.y;
    info["degree"] = static_cast<int>(m_graph->getEdgesFrom(nodeId).size());

    emit selectionChanged();
    emit selectionInfoReady(info);
    update();
    return info;
}

QVariantMap MapView::selectEdge(int source, int target) {
    QVariantMap info;
    if (!m_graph) return info;

    m_selectedNodeId = -1;
    m_selectedEdgeSource = source;
    m_selectedEdgeTarget = target;
    m_selectionDirty = true;
    m_geometryDirty = true;

    m_highlightedNodeIds.clear();
    m_highlightedNodeIds.insert(source);
    m_highlightedNodeIds.insert(target);
    m_nodeHighlightProgress = 0.0;

    m_animStartMs = QDateTime::currentMSecsSinceEpoch();
    m_animTimer->start();

    Node n1 = m_graph->getNode(source);
    Node n2 = m_graph->getNode(target);
    info["type"] = "edge";
    info["source"] = source;
    info["target"] = target;
    info["sourceX"] = n1.x;
    info["sourceY"] = n1.y;
    info["targetX"] = n2.x;
    info["targetY"] = n2.y;

    emit selectionChanged();
    emit selectionInfoReady(info);
    update();
    return info;
}

void MapView::clearSelection() {
    if (m_selectedNodeId == -1 && m_selectedEdgeSource == -1) return;
    m_selectedNodeId = -1;
    m_selectedEdgeSource = -1;
    m_selectedEdgeTarget = -1;
    m_selectionDirty = true;
    m_geometryDirty = true;
    m_highlightedNodeIds.clear();
    m_nodeHighlightProgress = 0.0;
    m_animTimer->stop();
    emit selectionChanged();
    update();
}

void MapView::onAnimTick() {
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    double elapsed = (now - m_animStartMs) / 1000.0;

    m_nodeHighlightProgress = std::min(elapsed / 0.45, 1.0);

    m_selectionDirty = true;
    update();

    if (m_nodeHighlightProgress >= 1.0)
        m_animTimer->stop();
}

void MapView::setZoom(double z) {
    if (m_zoom != z) {
        m_zoom = z;
        emit zoomChanged();
        update();
    }
}

void MapView::setOffset(const QPointF& o) {
    updateRange();
    QPointF clamped = o;
    applyOffsetBounds(clamped);
    if (m_offset != clamped) {
        m_offset = clamped;
        emit offsetChanged();
        update();
    }
}

void MapView::reclampOffset() {
    updateRange();
    QPointF clamped = m_offset;
    applyOffsetBounds(clamped);
    if (m_offset != clamped) {
        m_offset = clamped;
        emit offsetChanged();
    }
}

void MapView::applyOffsetBounds(QPointF& p) const {
    double worldW = m_range.maxX - m_range.minX;
    double worldH = m_range.maxY - m_range.minY;
    if (worldW <= 0 || worldH <= 0 || width() <= 0 || height() <= 0) return;

    double baseScale = std::min(width() / worldW, height() / worldH);
    double normX = worldW * baseScale / width();
    double normY = worldH * baseScale / height();

    p.setX(std::clamp(p.x(), normX * (0.5 - m_zoom), normX * 0.5));
    p.setY(std::clamp(p.y(), normY * (0.5 - m_zoom), normY * 0.5));
}

void MapView::addZoomVelocity(double delta) {
    double instant = std::pow(1.04, delta);
    m_zoom = std::clamp(m_zoom * instant, 0.1, 50.0);
    emit zoomChanged();
    reclampOffset();

    m_zoomVelocity += delta * 0.028;
    m_zoomVelocity = std::clamp(m_zoomVelocity, -0.22, 0.22);

    if (!m_momentumActive) {
        m_momentumActive = true;
        emit momentumActiveChanged();
    }
    m_lastMomentumMs = QDateTime::currentMSecsSinceEpoch();

    if (!m_momentumTimer->isActive())
        m_momentumTimer->start();

    update();
}

void MapView::onMomentumTick() {
    if (!m_momentumActive) {
        m_momentumTimer->stop();
        return;
    }

    qint64 now = QDateTime::currentMSecsSinceEpoch();
    double dt = std::max(0.0, static_cast<double>(now - m_lastMomentumMs) / 1000.0);
    m_lastMomentumMs = now;

    if (dt > 0.0 && dt < 0.5) {
        m_zoom *= (1.0 + m_zoomVelocity * dt * 60.0);
        m_zoom = std::clamp(m_zoom, 0.1, 50.0);
        reclampOffset();
        m_zoomVelocity *= std::exp(-8.66 * dt);

        if (std::abs(m_zoomVelocity) < 0.0003) {
            m_momentumActive = false;
            m_zoomVelocity = 0.0;
            m_momentumTimer->stop();
            emit momentumActiveChanged();
        }
    }

    emit zoomChanged();
    update();
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

QPointF MapView::mapFromScreen(double sx, double sy) const {
    double dx = m_range.maxX - m_range.minX;
    double dy = m_range.maxY - m_range.minY;
    if (dx <= 0) dx = 1;
    if (dy <= 0) dy = 1;

    double baseScale = std::min(width() / dx, height() / dy);
    double startX = (width()  - dx * baseScale) / 2.0;
    double startY = (height() - dy * baseScale) / 2.0;

    double rx = sx - startX - m_offset.x() * width();
    double ry = sy - startY - m_offset.y() * height();

    double tx = (rx / (baseScale * m_zoom)) + m_range.minX;
    double ty = (ry / (baseScale * m_zoom)) + m_range.minY;

    return QPointF(tx, ty);
}

QSGNode *MapView::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) {
    if (!m_graph) return oldNode;
    updateRange();
    updateDegreeOrder();

    QSGNode *container = oldNode;
    QSGGeometryNode *gridNode = nullptr;
    QSGTransformNode *mapNode = nullptr;

    if (!container) {
        container = new QSGNode();
        m_topologyDirty = true;
        m_geometryDirty = true;

        gridNode = new QSGGeometryNode();
        auto material = new QSGFlatColorMaterial();
        material->setColor(QColor(56, 210, 255, 15));
        gridNode->setMaterial(material);
        gridNode->setFlag(QSGNode::OwnsMaterial, true);
        
        auto geom = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 0);
        geom->setDrawingMode(QSGGeometry::DrawLines);
        geom->setLineWidth(1);
        gridNode->setGeometry(geom);
        gridNode->setFlag(QSGNode::OwnsGeometry, true);

        mapNode = new QSGTransformNode();

        container->appendChildNode(gridNode);
        container->appendChildNode(mapNode);
    } else {
        gridNode = static_cast<QSGGeometryNode *>(container->childAtIndex(0));
        mapNode = static_cast<QSGTransformNode *>(container->childAtIndex(1));
    }

    const auto& edges = m_graph->getAllEdges();
    const auto& nodes = m_graph->getAllNodes();
    int totalNodeCount = static_cast<int>(nodes.size());

    double worldW = m_range.maxX - m_range.minX;
    double worldH = m_range.maxY - m_range.minY;
    if (worldW <= 0) worldW = 1;
    if (worldH <= 0) worldH = 1;

    double baseScale     = std::min(width() / worldW, height() / worldH);
    double pixelsPerUnit = baseScale * m_zoom;
    float halfSize = static_cast<float>(std::clamp(pixelsPerUnit * 0.85, 3.2, 8.0));

    double startX = (width()  - worldW * baseScale) / 2.0;
    double startY = (height() - worldH * baseScale) / 2.0;

    // --- Rebuild Grid (only when parameters change meaningfully) ---
    double trueSpacing = 100.0 * baseScale * m_zoom;
    if (trueSpacing > 0) {
        while (trueSpacing > 160.0) trueSpacing /= 2.0;
        while (trueSpacing < 80.0) trueSpacing *= 2.0;
    } else {
        trueSpacing = 80.0;
    }

    double originX = startX + m_offset.x() * width() - m_range.minX * baseScale * m_zoom;
    double originY = startY + m_offset.y() * height() - m_range.minY * baseScale * m_zoom;

    bool gridChanged =
        m_cachedWidth != width() || m_cachedHeight != height() ||
        std::abs(m_cachedGridSpacing - trueSpacing) > 0.05 * trueSpacing ||
        std::abs(m_cachedGridOriginX - originX) > trueSpacing * 0.15 ||
        std::abs(m_cachedGridOriginY - originY) > trueSpacing * 0.15;

    if (gridChanged) {
        m_cachedGridSpacing = trueSpacing;
        m_cachedGridOriginX = originX;
        m_cachedGridOriginY = originY;

    double beginX = std::fmod(originX, trueSpacing);
    if (beginX < 0) beginX += trueSpacing;
    double beginY = std::fmod(originY, trueSpacing);
    if (beginY < 0) beginY += trueSpacing;

    int numX = static_cast<int>(width() / trueSpacing) + 2;
    int numY = static_cast<int>(height() / trueSpacing) + 2;

    QSGGeometry *gridGeom = gridNode->geometry();
    // i runs from -1 to numX-1 (which is numX + 1 iterations) for both X and Y.
    // Total geometric points needed: ((numX + 1) + (numY + 1)) * 2 = (numX + numY + 2) * 2.
    gridGeom->allocate((numX + numY + 2) * 2);
    QSGGeometry::Point2D *gridVertices = gridGeom->vertexDataAsPoint2D();
    
    int gIdx = 0;
    for (int i = -1; i < numX; ++i) {
        double x = beginX + i * trueSpacing;
        gridVertices[gIdx++].set(x, 0);
        gridVertices[gIdx++].set(x, height());
    }
    for (int i = -1; i < numY; ++i) {
        double y = beginY + i * trueSpacing;
        gridVertices[gIdx++].set(0, y);
        gridVertices[gIdx++].set(width(), y);
    }
    gridNode->markDirty(QSGNode::DirtyGeometry);
    } // gridChanged

    if (m_cachedWidth != width() || m_cachedHeight != height()) {
        m_cachedWidth = width();
        m_cachedHeight = height();
        m_geometryDirty = true;
        m_topologyDirty = true;
    }

    if (m_bakedZoom < 0.0 || m_zoom / m_bakedZoom < 0.55 || m_zoom / m_bakedZoom > 1.7) {
        m_geometryDirty = true;
    }

    int visibleCount = totalNodeCount;
    if (m_lodEnabled) {
        double lodRatio = (m_bakedLodZoom > 0.0) ? (m_zoom / m_bakedLodZoom) : 999.0;
        if (lodRatio < 0.6 || lodRatio > 1.6 || m_cachedVisibleCount < 0) {
            const int MIN_VISIBLE = 200;
            // Normalize zoom to [0, 1] so that lodIntensity always controls
            // the reduction consistently: Soft → more nodes, Aggressive → fewer nodes,
            // at every zoom level (zoom ∈ [0.1, 50.0], log scale for natural feel).
            double zoomNorm = std::clamp(
                std::log(m_zoom / 0.1) / std::log(50.0 / 0.1), 0.0, 1.0);
            int targetVisible = static_cast<int>(
                MIN_VISIBLE + (totalNodeCount - MIN_VISIBLE) * std::pow(zoomNorm, m_lodIntensity));
            int newVisible = std::min(targetVisible, static_cast<int>(m_nodesByDegree.size()));

            m_bakedLodZoom = m_zoom;

            if (std::abs(m_cachedVisibleCount - newVisible) > std::max(100, m_cachedVisibleCount / 5)) {
                m_cachedVisibleCount = newVisible;
                m_topologyDirty = true;
            }
        }
        visibleCount = m_cachedVisibleCount;
    }

    if (m_hoveredEdgeSource != m_cachedHoverLo || m_hoveredEdgeTarget != m_cachedHoverHi) {
        m_topologyDirty = true; // Need to map hovered edge to macroscopic nodes
        m_geometryDirty = true;
    }

    if (m_geometryDirty || m_topologyDirty || m_styleDirty) {
        m_bakedZoom = m_zoom;
    }

    double s = 1.0;
    if (m_bakedZoom > 0.0) {
        s = m_zoom / m_bakedZoom;
    }

    // 更新平移和缩放矩阵，无需重建几何体
    QMatrix4x4 matrix;
    matrix.translate(m_offset.x() * width(), m_offset.y() * height());
    matrix.translate(startX * (1.0 - s), startY * (1.0 - s));
    matrix.scale(s, s);
    mapNode->setMatrix(matrix);

    // Selection-only dirty → rebuild overlay, skip base geometry (animation frames)
    if (!m_topologyDirty && !m_geometryDirty && !m_styleDirty && m_selectionDirty) {
        rebuildSelectionOverlay(mapNode, halfSize, pixelsPerUnit);
        m_selectionDirty = false;
        return container;
    }

    if (!m_topologyDirty && !m_geometryDirty && !m_styleDirty && !m_selectionDirty) {
        return container; // 只有 offset 改变或轻微缩放时，直接返回，实现 0 成本高帧率
    }

    if (m_topologyDirty) {
        m_cachedLogicalEdges.clear();

        int maxId = 0;
        for (const auto& n : nodes) {
            if (n.Node_id > maxId) maxId = n.Node_id;
        }

        std::vector<int> nodeOwner(maxId + 1, -1);
        std::queue<int> bfsQ;

        for (int k = 0; k < visibleCount; ++k) {
            int idx = m_nodesByDegree[k];
            int vid = nodes[idx].Node_id;
            nodeOwner[vid] = vid;
            bfsQ.push(vid);
        }
        // Force-include selected node in BFS seeds (even if below LOD threshold)
        if (m_selectedNodeId != -1 && m_selectedNodeId <= maxId
            && nodeOwner[m_selectedNodeId] == -1) {
            nodeOwner[m_selectedNodeId] = m_selectedNodeId;
            bfsQ.push(m_selectedNodeId);
        }

        while (!bfsQ.empty()) {
            int curId = bfsQ.front(); bfsQ.pop();
            int owner = nodeOwner[curId];
            for (const auto& e : m_graph->getEdgesFrom(curId)) {
                if (e.target <= maxId && nodeOwner[e.target] == -1) {
                    nodeOwner[e.target] = owner;
                    bfsQ.push(e.target);
                }
            }
        }

        struct TempEdge { uint64_t key; int capacity; double ratio; };
        std::vector<TempEdge> tempEdges;
        tempEdges.reserve(edges.size());

        for (const auto& edge : edges) {
            if (edge.source > maxId || edge.target > maxId) continue;
            int ownerSrc = nodeOwner[edge.source];
            int ownerDst = nodeOwner[edge.target];
            if (ownerSrc == -1 || ownerDst == -1 || ownerSrc == ownerDst) continue;

            int lo = std::min(ownerSrc, ownerDst);
            int hi = std::max(ownerSrc, ownerDst);
            uint64_t key = (static_cast<uint64_t>(hi) << 32) | static_cast<uint64_t>(lo);
            double ratio = (edge.capacity > 0) ? static_cast<double>(edge.currentCars) / edge.capacity : 0;
            tempEdges.push_back({key, static_cast<int>(edge.capacity), ratio});
        }

        std::sort(tempEdges.begin(), tempEdges.end(), [](const TempEdge& a, const TempEdge& b){
            return a.key < b.key;
        });

        if (!tempEdges.empty()) {
            uint64_t lastKey = tempEdges[0].key;
            int maxCap = tempEdges[0].capacity;
            double maxRatio = tempEdges[0].ratio;
            for (size_t i = 1; i < tempEdges.size(); ++i) {
                if (tempEdges[i].key == lastKey) {
                    if (tempEdges[i].capacity > maxCap) maxCap = tempEdges[i].capacity;
                    if (tempEdges[i].ratio > maxRatio) maxRatio = tempEdges[i].ratio;
                } else {
                    m_cachedLogicalEdges.push_back({static_cast<int>(lastKey & 0xFFFFFFFF), static_cast<int>(lastKey >> 32), maxRatio, maxCap});
                    lastKey = tempEdges[i].key;
                    maxCap = tempEdges[i].capacity;
                    maxRatio = tempEdges[i].ratio;
                }
            }
            m_cachedLogicalEdges.push_back({static_cast<int>(lastKey & 0xFFFFFFFF), static_cast<int>(lastKey >> 32), maxRatio, maxCap});
        }

        m_cachedHoverLo = m_hoveredEdgeSource;
        m_cachedHoverHi = m_hoveredEdgeTarget;
        
        m_topologyDirty = false;
        m_geometryDirty = true;
    }

    if (m_geometryDirty || m_styleDirty) {
        mapNode->removeAllChildNodes();
        if (m_styleDirty && m_nodeTexture) {
            delete m_nodeTexture;
            m_nodeTexture = nullptr;
        }
        m_geometryDirty = false;
        m_styleDirty = false;

        double startX = (width()  - worldW * baseScale) / 2.0;
        double startY = (height() - worldH * baseScale) / 2.0;

        auto getBasePos = [&](double x, double y) {
            double sx = startX + (x - m_range.minX) * baseScale * m_zoom;
            double sy = startY + (y - m_range.minY) * baseScale * m_zoom;
            return QPointF(sx, sy);
        };

        int hoverLoMapping = -1, hoverHiMapping = -1;
        if (m_cachedHoverLo != -1 && m_cachedHoverHi != -1) {
            // BFS nodeOwner needs to be checked, but we did this during mapping.
            // Since we threw away nodeOwner, we can just find if an edge matches cached hover
            hoverLoMapping = m_cachedHoverLo;
            hoverHiMapping = m_cachedHoverHi;
        }

        struct DrawEdge {
            QPointF p1, p2;
            double ratio;
            int capacity;
            bool isHovered;
            double baseWidth;
            int u, v;
        };

        auto baseEdgeWidth = [&](int capacity) {
            double cap = std::max(1, capacity);
            double width = std::sqrt(cap) * pixelsPerUnit * 0.65;
            return std::clamp(width, 1.8, 9.0);
        };

        std::vector<DrawEdge> drawEdges;
        drawEdges.reserve(m_cachedLogicalEdges.size());
        for (const auto& logEdge : m_cachedLogicalEdges) {
            Node n1 = m_graph->getNode(logEdge.u);
            Node n2 = m_graph->getNode(logEdge.v);
            QPointF p1 = getBasePos(n1.x, n1.y);
            QPointF p2 = getBasePos(n2.x, n2.y);
            
            // To simplify hover logic, just match ID. 
            // In a real scenario we'd track the mapped macroscopic owner.
            bool isHovered = (logEdge.u == hoverLoMapping && logEdge.v == hoverHiMapping) || 
                             (logEdge.v == hoverLoMapping && logEdge.u == hoverHiMapping);
                             
            double baseWidth = baseEdgeWidth(logEdge.capacity);
            drawEdges.push_back({p1, p2, logEdge.ratio, logEdge.capacity, isHovered, baseWidth, logEdge.u, logEdge.v});
        }

        std::sort(drawEdges.begin(), drawEdges.end(), [](const DrawEdge& a, const DrawEdge& b) {
            return a.baseWidth > b.baseWidth;
        });

        auto clamp01 = [](double v) {
            return std::clamp(v, 0.0, 1.0);
        };

        auto mixColor = [](const QColor& a, const QColor& b, double t) {
            double tt = std::clamp(t, 0.0, 1.0);
            QColor out;
            out.setRedF(a.redF() + (b.redF() - a.redF()) * tt);
            out.setGreenF(a.greenF() + (b.greenF() - a.greenF()) * tt);
            out.setBlueF(a.blueF() + (b.blueF() - a.blueF()) * tt);
            out.setAlphaF(a.alphaF() + (b.alphaF() - a.alphaF()) * tt);
            return out;
        };

        auto edgeRampColor = [&](double ratio) {
            double t = clamp01(ratio);
            if (t <= m_edgeMidRatio) {
                double denom = std::max(1e-6, m_edgeMidRatio);
                return mixColor(m_edgeCoolColor, m_edgeMidColor, t / denom);
            }
            double denom = std::max(1e-6, 1.0 - m_edgeMidRatio);
            return mixColor(m_edgeMidColor, m_edgeWarmColor, (t - m_edgeMidRatio) / denom);
        };

        auto applyAlpha = [&](const QColor& c, double alphaScale) {
            QColor out = c;
            out.setAlphaF(clamp01(c.alphaF() * alphaScale));
            return out;
        };

        const int MAX_ITEMS_PER_CHUNK = 10000;
        
        auto appendEdgeLayer = [&](int chunkStart, int chunkSize, double widthScale, double alphaScale, double glowTint, bool isGlow) {
            QSGGeometryNode *edgeNode = new QSGGeometryNode();
            auto *material = new QSGVertexColorMaterial();
            material->setFlag(QSGMaterial::Blending, true);
            edgeNode->setMaterial(material);
            edgeNode->setFlag(QSGNode::OwnsMaterial);
            edgeNode->setFlag(QSGNode::OwnsGeometry);

            int vertsPerEdge = 6;
            QSGGeometry *edgeGeom = new QSGGeometry(
                QSGGeometry::defaultAttributes_ColoredPoint2D(), chunkSize * vertsPerEdge);
            edgeGeom->setDrawingMode(QSGGeometry::DrawTriangles);
            auto *eVerts = edgeGeom->vertexDataAsColoredPoint2D();

            for (int i = 0; i < chunkSize; ++i) {
                const auto& edge = drawEdges[chunkStart + i];
                double dx = edge.p2.x() - edge.p1.x();
                double dy = edge.p2.y() - edge.p1.y();
                double len = std::hypot(dx, dy);

                int base = i * vertsPerEdge;
                if (len < 1e-5) {
                    for (int j = 0; j < vertsPerEdge; ++j) eVerts[base + j].set(0, 0, 0, 0, 0, 0);
                    continue;
                }

                double nx = dy / len;
                double ny = -dx / len;

                double width = edge.baseWidth;
                if (edge.isHovered) width *= 1.35;
                width *= widthScale;

                double wx = nx * width / 2.0;
                double wy = ny * width / 2.0;

                QColor baseColor = edge.isHovered ? m_edgeHoverColor : edgeRampColor(edge.ratio);
                if (glowTint > 0.0) baseColor = mixColor(baseColor, QColor(255, 255, 255), glowTint);
                QColor finalColor = applyAlpha(baseColor, alphaScale);

                // Dim non-selected / non-connected edges when selection is active
                if (!isGlow && (m_selectedNodeId != -1 || m_selectedEdgeSource != -1)) {
                    bool isSelEdge = (edge.u == m_selectedEdgeSource && edge.v == m_selectedEdgeTarget)
                                  || (edge.v == m_selectedEdgeSource && edge.u == m_selectedEdgeTarget);
                    bool isConnected = (m_selectedNodeId != -1)
                        && (edge.u == m_selectedNodeId || edge.v == m_selectedNodeId);
                    if (!isSelEdge && !isConnected) {
                        finalColor.setAlphaF(finalColor.alphaF() * 0.18);
                    } else if (isConnected && !isSelEdge) {
                        finalColor.setAlphaF(finalColor.alphaF() * 0.55);
                    }
                }

                unsigned char r = static_cast<unsigned char>(finalColor.red());
                unsigned char g = static_cast<unsigned char>(finalColor.green());
                unsigned char b = static_cast<unsigned char>(finalColor.blue());
                unsigned char a = static_cast<unsigned char>(finalColor.alpha());

                eVerts[base + 0].set(edge.p1.x() - wx, edge.p1.y() - wy, r, g, b, a);
                eVerts[base + 1].set(edge.p1.x() + wx, edge.p1.y() + wy, r, g, b, a);
                eVerts[base + 2].set(edge.p2.x() - wx, edge.p2.y() - wy, r, g, b, a);
                eVerts[base + 3].set(edge.p2.x() - wx, edge.p2.y() - wy, r, g, b, a);
                eVerts[base + 4].set(edge.p1.x() + wx, edge.p1.y() + wy, r, g, b, a);
                eVerts[base + 5].set(edge.p2.x() + wx, edge.p2.y() + wy, r, g, b, a);
            }
            edgeNode->setGeometry(edgeGeom);
            mapNode->appendChildNode(edgeNode);
        };

        int ve = static_cast<int>(drawEdges.size());
        for (int chunkStart = 0; chunkStart < ve; chunkStart += MAX_ITEMS_PER_CHUNK) {
            int chunkEnd = std::min(ve, chunkStart + MAX_ITEMS_PER_CHUNK);
            int chunkSize = chunkEnd - chunkStart;
            appendEdgeLayer(chunkStart, chunkSize, m_edgeGlowWidthScale, m_edgeGlowAlpha, 0.35, true);
            appendEdgeLayer(chunkStart, chunkSize, 1.0, m_edgeCoreAlpha, 0.0, false);
        }

        if (!m_nodeTexture) {
            const int texSize = 64;
            QImage image(texSize, texSize, QImage::Format_RGBA8888);
            image.fill(Qt::transparent);
            QPainter p(&image);
            p.setRenderHint(QPainter::Antialiasing);

            float cx = texSize / 2.0f;
            float cy = texSize / 2.0f;

            // Draw Glow
            QRadialGradient glowGrad(cx, cy, cx);
            QColor innerGlow = m_nodeGlowColor;
            innerGlow.setAlphaF(clamp01(m_nodeGlowAlpha));
            QColor midGlow = m_nodeGlowColor;
            midGlow.setAlphaF(clamp01(m_nodeGlowAlpha * 0.4));
            QColor outerGlow = m_nodeGlowColor;
            outerGlow.setAlphaF(0.0);

            glowGrad.setColorAt(0.0, innerGlow);
            glowGrad.setColorAt(0.5, midGlow);
            glowGrad.setColorAt(1.0, Qt::transparent);
            p.setBrush(glowGrad);
            p.setPen(Qt::NoPen);
            p.drawEllipse(0, 0, texSize, texSize);

            // Draw Core
            float coreRad = cx / m_nodeGlowSizeScale; 
            QRadialGradient coreGrad(cx, cy, coreRad);
            QColor coreColor = m_nodeCoreColor;
            coreColor.setAlphaF(clamp01(m_nodeCoreAlpha));
            
            coreGrad.setColorAt(0.0, coreColor);
            coreGrad.setColorAt(0.8, coreColor);
            coreGrad.setColorAt(1.0, Qt::transparent);
            p.setBrush(coreGrad);
            p.setPen(Qt::NoPen);
            p.drawEllipse(QRectF(cx - coreRad, cy - coreRad, coreRad * 2, coreRad * 2));
            
            p.end();
            m_nodeTexture = window()->createTextureFromImage(image);
        }

        constexpr int NODE_VERTS_PER = 6;
        const int MAX_NODE_ITEMS_PER_CHUNK = std::max(1, 60000 / NODE_VERTS_PER);

        auto appendNodeLayer = [&](int chunkStart, int chunkSize, float sizeScale) {
            QSGGeometryNode *pointNode = new QSGGeometryNode();
            auto *material = new QSGTextureMaterial();
            material->setTexture(m_nodeTexture);
            material->setFlag(QSGMaterial::Blending, true);
            pointNode->setMaterial(material);
            pointNode->setFlag(QSGNode::OwnsMaterial);
            pointNode->setFlag(QSGNode::OwnsGeometry);

            QSGGeometry *pointGeom = new QSGGeometry(
                QSGGeometry::defaultAttributes_TexturedPoint2D(), chunkSize * NODE_VERTS_PER);
            pointGeom->setDrawingMode(QSGGeometry::DrawTriangles);
            auto *pVerts = pointGeom->vertexDataAsTexturedPoint2D();

            float radius = halfSize * sizeScale;
            for (int i = 0; i < chunkSize; ++i) {
                int idx = m_nodesByDegree[chunkStart + i];
                QPointF p = getBasePos(nodes[idx].x, nodes[idx].y);
                float px  = static_cast<float>(p.x());
                float py  = static_cast<float>(p.y());

                int base = i * NODE_VERTS_PER;
                float x0 = px - radius;
                float y0 = py - radius;
                float x1 = px + radius;
                float y1 = py + radius;

                pVerts[base + 0].set(x0, y0, 0.0f, 0.0f);
                pVerts[base + 1].set(x1, y0, 1.0f, 0.0f);
                pVerts[base + 2].set(x0, y1, 0.0f, 1.0f);
                pVerts[base + 3].set(x0, y1, 0.0f, 1.0f);
                pVerts[base + 4].set(x1, y0, 1.0f, 0.0f);
                pVerts[base + 5].set(x1, y1, 1.0f, 1.0f);
            }
            pointNode->setGeometry(pointGeom);
            mapNode->appendChildNode(pointNode);
        };

        for (int chunkStart = 0; chunkStart < visibleCount; chunkStart += MAX_NODE_ITEMS_PER_CHUNK) {
            int chunkEnd = std::min(visibleCount, chunkStart + MAX_NODE_ITEMS_PER_CHUNK);
            int chunkSize = chunkEnd - chunkStart;
            appendNodeLayer(chunkStart, chunkSize, static_cast<float>(m_nodeGlowSizeScale));
        }

        m_baseChildCount = mapNode->childCount();
    }

    // Rebuild selection overlay on top of base geometry when needed
    if (m_selectionDirty) {
        rebuildSelectionOverlay(mapNode, halfSize, pixelsPerUnit);
        m_selectionDirty = false;
    }

    return container;
}

void MapView::rebuildSelectionOverlay(QSGTransformNode* mapNode, float halfSize, double pixelsPerUnit) {
    // Remove previous overlay children
    while (mapNode->childCount() > m_baseChildCount)
        mapNode->removeChildNode(mapNode->childAtIndex(mapNode->childCount() - 1));

    auto getBasePos = [&](double x, double y) {
        double worldW = m_range.maxX - m_range.minX;
        double worldH = m_range.maxY - m_range.minY;
        if (worldW <= 0) worldW = 1;
        if (worldH <= 0) worldH = 1;
        double baseScale = std::min(width() / worldW, height() / worldH);
        double startX = (width()  - worldW * baseScale) / 2.0;
        double startY = (height() - worldH * baseScale) / 2.0;
        return QPointF(
            startX + (x - m_range.minX) * baseScale * m_zoom,
            startY + (y - m_range.minY) * baseScale * m_zoom);
    };

    // ── 1. Selected node glow ──
    if (m_selectedNodeId != -1) {
        Node n = m_graph->getNode(m_selectedNodeId);
        QPointF p = getBasePos(n.x, n.y);
        float px = static_cast<float>(p.x());
        float py = static_cast<float>(p.y());
        float r = halfSize * 1.8f;

        QSGGeometryNode* sn = new QSGGeometryNode();
        auto* mat = new QSGTextureMaterial();
        mat->setTexture(m_nodeTexture);
        mat->setFlag(QSGMaterial::Blending, true);
        sn->setMaterial(mat);
        sn->setFlag(QSGNode::OwnsMaterial);
        sn->setFlag(QSGNode::OwnsGeometry);

        QSGGeometry* sg = new QSGGeometry(
            QSGGeometry::defaultAttributes_TexturedPoint2D(), 6);
        sg->setDrawingMode(QSGGeometry::DrawTriangles);
        auto* sv = sg->vertexDataAsTexturedPoint2D();
        sv[0].set(px - r, py - r, 0.0f, 0.0f);
        sv[1].set(px + r, py - r, 1.0f, 0.0f);
        sv[2].set(px - r, py + r, 0.0f, 1.0f);
        sv[3].set(px - r, py + r, 0.0f, 1.0f);
        sv[4].set(px + r, py - r, 1.0f, 0.0f);
        sv[5].set(px + r, py + r, 1.0f, 1.0f);
        sn->setGeometry(sg);
        mapNode->appendChildNode(sn);
    }

    // ── 2. Selected edge highlight ──
    if (m_selectedEdgeSource != -1 && m_selectedEdgeTarget != -1) {
        Node n1 = m_graph->getNode(m_selectedEdgeSource);
        Node n2 = m_graph->getNode(m_selectedEdgeTarget);
        QPointF p1 = getBasePos(n1.x, n1.y);
        QPointF p2 = getBasePos(n2.x, n2.y);
        double dx = p2.x() - p1.x();
        double dy = p2.y() - p1.y();
        double len = std::hypot(dx, dy);
        if (len > 1e-5) {
            double nx = dy / len;
            double ny = -dx / len;
            double w = std::clamp(pixelsPerUnit * 2.5, 2.5, 10.0);
            double wx = nx * w / 2.0;
            double wy = ny * w / 2.0;

            QSGGeometryNode* en = new QSGGeometryNode();
            auto* mat = new QSGVertexColorMaterial();
            mat->setFlag(QSGMaterial::Blending, true);
            en->setMaterial(mat);
            en->setFlag(QSGNode::OwnsMaterial);
            en->setFlag(QSGNode::OwnsGeometry);

            QSGGeometry* eg = new QSGGeometry(
                QSGGeometry::defaultAttributes_ColoredPoint2D(), 6);
            eg->setDrawingMode(QSGGeometry::DrawTriangles);
            auto* ev = eg->vertexDataAsColoredPoint2D();
            QColor accent = m_selectionAccent;
            unsigned char ar = static_cast<unsigned char>(accent.red());
            unsigned char ag = static_cast<unsigned char>(accent.green());
            unsigned char ab = static_cast<unsigned char>(accent.blue());
            unsigned char aa = 240;
            ev[0].set(p1.x() - wx, p1.y() - wy, ar, ag, ab, aa);
            ev[1].set(p1.x() + wx, p1.y() + wy, ar, ag, ab, aa);
            ev[2].set(p2.x() - wx, p2.y() - wy, ar, ag, ab, aa);
            ev[3].set(p2.x() - wx, p2.y() - wy, ar, ag, ab, aa);
            ev[4].set(p1.x() + wx, p1.y() + wy, ar, ag, ab, aa);
            ev[5].set(p2.x() + wx, p2.y() + wy, ar, ag, ab, aa);
            en->setGeometry(eg);
            mapNode->appendChildNode(en);
        }
    }

    // ── 3. Edge-click endpoint highlights (animated node glow) ──
    if (!m_highlightedNodeIds.empty()) {
        double et = m_nodeHighlightProgress;
        double eased = et < 1.0
            ? 1.0 + 0.8 * (1.0 - std::pow(1.0 - et, 3.0)) * (1.0 + 0.7 * std::sin(et * 3.1415926535))
            : 1.8;
        float hlRadius = halfSize * static_cast<float>(eased);

        QSGGeometryNode* hn = new QSGGeometryNode();
        auto* mat = new QSGTextureMaterial();
        mat->setTexture(m_nodeTexture);
        mat->setFlag(QSGMaterial::Blending, true);
        hn->setMaterial(mat);
        hn->setFlag(QSGNode::OwnsMaterial);
        hn->setFlag(QSGNode::OwnsGeometry);

        int hc = static_cast<int>(m_highlightedNodeIds.size());
        QSGGeometry* hg = new QSGGeometry(
            QSGGeometry::defaultAttributes_TexturedPoint2D(), hc * 6);
        hg->setDrawingMode(QSGGeometry::DrawTriangles);
        auto* hv = hg->vertexDataAsTexturedPoint2D();

        int vi = 0;
        for (int nid : m_highlightedNodeIds) {
            Node nd = m_graph->getNode(nid);
            QPointF p = getBasePos(nd.x, nd.y);
            float px = static_cast<float>(p.x());
            float py = static_cast<float>(p.y());
            hv[vi + 0].set(px - hlRadius, py - hlRadius, 0.0f, 0.0f);
            hv[vi + 1].set(px + hlRadius, py - hlRadius, 1.0f, 0.0f);
            hv[vi + 2].set(px - hlRadius, py + hlRadius, 0.0f, 1.0f);
            hv[vi + 3].set(px - hlRadius, py + hlRadius, 0.0f, 1.0f);
            hv[vi + 4].set(px + hlRadius, py - hlRadius, 1.0f, 0.0f);
            hv[vi + 5].set(px + hlRadius, py + hlRadius, 1.0f, 1.0f);
            vi += 6;
        }
        hn->setGeometry(hg);
        mapNode->appendChildNode(hn);
    }
}
