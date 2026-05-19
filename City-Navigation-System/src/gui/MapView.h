#pragma once

#include <QQuickItem>
#include <QSGNode>
#include <QSGGeometryNode>
#include <QSGFlatColorMaterial>
#include <QSGVertexColorMaterial>
#include <QVariant>
#include <QVariantMap>
#include <QPointF>
#include <QColor>
#include <QTimer>
#include <QElapsedTimer>
#include <QFutureWatcher>
#include "../core/DataModel/Graph.h"

#include <QSGTransformNode>

class MapView : public QQuickItem {
    Q_OBJECT
    Q_PROPERTY(Graph* graph READ graph WRITE setGraph NOTIFY graphChanged)
    // 缩放和平移属性，供 QML 交互使用
    Q_PROPERTY(double zoom READ zoom WRITE setZoom NOTIFY zoomChanged)
    Q_PROPERTY(QPointF offset READ offset WRITE setOffset NOTIFY offsetChanged)
    // LOD 细节倍率：控制显示的节点密度（0.0 = 全量显示, 3.0 = 极度稀疏）
    Q_PROPERTY(double lodIntensity READ lodIntensity WRITE setLodIntensity NOTIFY lodIntensityChanged)
    // 是否随缩放自动调整 LOD
    Q_PROPERTY(bool lodAutoZoom READ lodAutoZoom WRITE setLodAutoZoom NOTIFY lodAutoZoomChanged)
// ... rest of the properties are unchanged ...
    Q_PROPERTY(int hoveredEdgeSource READ hoveredEdgeSource WRITE setHoveredEdgeSource NOTIFY hoveredEdgeChanged)
    Q_PROPERTY(int hoveredEdgeTarget READ hoveredEdgeTarget WRITE setHoveredEdgeTarget NOTIFY hoveredEdgeChanged)

    Q_PROPERTY(QColor edgeCoolColor READ edgeCoolColor WRITE setEdgeCoolColor NOTIFY styleChanged)
    Q_PROPERTY(QColor edgeMidColor READ edgeMidColor WRITE setEdgeMidColor NOTIFY styleChanged)
    Q_PROPERTY(QColor edgeWarmColor READ edgeWarmColor WRITE setEdgeWarmColor NOTIFY styleChanged)
    Q_PROPERTY(QColor edgeHoverColor READ edgeHoverColor WRITE setEdgeHoverColor NOTIFY styleChanged)
    Q_PROPERTY(double edgeMidRatio READ edgeMidRatio WRITE setEdgeMidRatio NOTIFY styleChanged)
    Q_PROPERTY(double edgeCoreAlpha READ edgeCoreAlpha WRITE setEdgeCoreAlpha NOTIFY styleChanged)
    Q_PROPERTY(double edgeGlowAlpha READ edgeGlowAlpha WRITE setEdgeGlowAlpha NOTIFY styleChanged)
    Q_PROPERTY(double edgeGlowWidthScale READ edgeGlowWidthScale WRITE setEdgeGlowWidthScale NOTIFY styleChanged)
    Q_PROPERTY(QColor virtualEdgeColor READ virtualEdgeColor WRITE setVirtualEdgeColor NOTIFY styleChanged)

    Q_PROPERTY(QColor nodeCoreColor READ nodeCoreColor WRITE setNodeCoreColor NOTIFY styleChanged)
    Q_PROPERTY(QColor nodeGlowColor READ nodeGlowColor WRITE setNodeGlowColor NOTIFY styleChanged)
    Q_PROPERTY(double nodeCoreAlpha READ nodeCoreAlpha WRITE setNodeCoreAlpha NOTIFY styleChanged)
    Q_PROPERTY(double nodeGlowAlpha READ nodeGlowAlpha WRITE setNodeGlowAlpha NOTIFY styleChanged)
    Q_PROPERTY(double nodeGlowSizeScale READ nodeGlowSizeScale WRITE setNodeGlowSizeScale NOTIFY styleChanged)
    Q_PROPERTY(bool momentumActive READ momentumActive NOTIFY momentumActiveChanged)

    // Selection state
    Q_PROPERTY(int selectedNodeId READ selectedNodeId NOTIFY selectionChanged)
    Q_PROPERTY(int selectedEdgeSource READ selectedEdgeSource NOTIFY selectionChanged)
    Q_PROPERTY(int selectedEdgeTarget READ selectedEdgeTarget NOTIFY selectionChanged)
    Q_PROPERTY(QString selectionMode READ selectionMode NOTIFY selectionChanged)
    Q_PROPERTY(QColor selectionAccent READ selectionAccent WRITE setSelectionAccent NOTIFY styleChanged)

    Q_PROPERTY(int edgeViewMode READ edgeViewMode WRITE setEdgeViewMode NOTIFY edgeViewModeChanged)
    Q_PROPERTY(bool routeMode READ routeMode WRITE setRouteMode NOTIFY routeModeChanged)
    Q_PROPERTY(int routeStartNodeId READ routeStartNodeId NOTIFY routeChanged)
    Q_PROPERTY(int routeEndNodeId READ routeEndNodeId NOTIFY routeChanged)
    Q_PROPERTY(QColor pathHighlightColor READ pathHighlightColor WRITE setPathHighlightColor NOTIFY styleChanged)
    Q_PROPERTY(bool hasRangeHighlight READ hasRangeHighlight NOTIFY rangeHighlightChanged)

public:
    explicit MapView(QQuickItem *parent = nullptr);
    ~MapView() override;

    Graph* graph() const { return m_graph; }
    void setGraph(Graph* graph);
    Q_INVOKABLE void refresh();

    Q_INVOKABLE QVariantMap hitTestNode(const QPointF& screenPos, double tolerance = 5.0) const;
    Q_INVOKABLE QVariantMap hitTestEdge(const QPointF& screenPos, double tolerance = 5.0) const;

    Q_INVOKABLE QVariantMap selectNode(int nodeId);
    Q_INVOKABLE QVariantMap selectEdge(int source, int target);
    Q_INVOKABLE void clearSelection();

    int edgeViewMode() const { return m_edgeViewMode; }
    void setEdgeViewMode(int mode);

    bool routeMode() const { return m_routeMode; }
    void setRouteMode(bool active);
    int routeStartNodeId() const { return m_routeStartNodeId; }
    int routeEndNodeId() const { return m_routeEndNodeId; }
    Q_INVOKABLE void selectRouteNode(int nodeId);
    Q_INVOKABLE void clearRoute();
    Q_INVOKABLE QVariantMap getPathInfo() const;

    bool hasRangeHighlight() const { return !m_rangeHighlightNodeIds.empty(); }
    Q_INVOKABLE void highlightNearestNodes(double x, double y);
    Q_INVOKABLE void clearRangeHighlight();

    QColor pathHighlightColor() const { return m_pathHighlightColor; }
    void setPathHighlightColor(const QColor& c) {
        if (m_pathHighlightColor != c) { m_pathHighlightColor = c; m_selectionDirty = true; emit styleChanged(); update(); }
    }

    Q_INVOKABLE void addZoomVelocity(double delta);
    Q_INVOKABLE void zoomAtPoint(double delta, double mouseX, double mouseY);
    void reclampOffset();
    void applyOffsetBounds(QPointF& p) const;

    bool momentumActive() const { return m_momentumActive; }

    int selectedNodeId() const { return m_selectedNodeId; }
    int selectedEdgeSource() const { return m_selectedEdgeSource; }
    int selectedEdgeTarget() const { return m_selectedEdgeTarget; }
    QString selectionMode() const {
        if (m_selectedNodeId != -1) return QStringLiteral("node");
        if (m_selectedEdgeSource != -1) return QStringLiteral("edge");
        return QStringLiteral("none");
    }
    QColor selectionAccent() const { return m_selectionAccent; }
    void setSelectionAccent(const QColor& c) { m_selectionAccent = c; }

    double zoom() const { return m_zoom; }
    void setZoom(double z);

    QPointF offset() const { return m_offset; }
    void setOffset(const QPointF& o);

    double lodIntensity() const { return m_lodIntensity; }
    void setLodIntensity(double v) {
        double clamped = std::clamp(v, 0.0, 3.0);
        if (!qFuzzyCompare(m_lodIntensity, clamped)) {
            m_lodIntensity = clamped;
            m_bakedLodZoom = -1.0;
            emit lodIntensityChanged();
            update();
        }
    }

    bool lodAutoZoom() const { return m_lodAutoZoom; }
    void setLodAutoZoom(bool v) {
        if (m_lodAutoZoom != v) { m_lodAutoZoom = v; m_bakedLodZoom = -1.0; emit lodAutoZoomChanged(); update(); }
    }

    int hoveredEdgeSource() const { return m_hoveredEdgeSource; }
    void setHoveredEdgeSource(int src) {
        if (m_hoveredEdgeSource != src) {
            m_hoveredEdgeSource = src;
            m_hoverDirty = true;
            emit hoveredEdgeChanged();
            update();
        }
    }

    int hoveredEdgeTarget() const { return m_hoveredEdgeTarget; }
    void setHoveredEdgeTarget(int target) {
        if (m_hoveredEdgeTarget != target) {
            m_hoveredEdgeTarget = target;
            m_hoverDirty = true;
            emit hoveredEdgeChanged();
            update();
        }
    }

    QColor edgeCoolColor() const { return m_edgeCoolColor; }
    void setEdgeCoolColor(const QColor& c) {
        if (m_edgeCoolColor != c) { m_edgeCoolColor = c; m_styleDirty = true; emit styleChanged(); update(); }
    }

    QColor edgeMidColor() const { return m_edgeMidColor; }
    void setEdgeMidColor(const QColor& c) {
        if (m_edgeMidColor != c) { m_edgeMidColor = c; m_styleDirty = true; emit styleChanged(); update(); }
    }

    QColor edgeWarmColor() const { return m_edgeWarmColor; }
    void setEdgeWarmColor(const QColor& c) {
        if (m_edgeWarmColor != c) { m_edgeWarmColor = c; m_styleDirty = true; emit styleChanged(); update(); }
    }

    QColor edgeHoverColor() const { return m_edgeHoverColor; }
    void setEdgeHoverColor(const QColor& c) {
        if (m_edgeHoverColor != c) { m_edgeHoverColor = c; m_styleDirty = true; emit styleChanged(); update(); }
    }

    double edgeMidRatio() const { return m_edgeMidRatio; }
    void setEdgeMidRatio(double v) {
        if (!qFuzzyCompare(m_edgeMidRatio, v)) { m_edgeMidRatio = v; m_styleDirty = true; emit styleChanged(); update(); }
    }

    double edgeCoreAlpha() const { return m_edgeCoreAlpha; }
    void setEdgeCoreAlpha(double v) {
        if (!qFuzzyCompare(m_edgeCoreAlpha, v)) { m_edgeCoreAlpha = v; m_styleDirty = true; emit styleChanged(); update(); }
    }

    double edgeGlowAlpha() const { return m_edgeGlowAlpha; }
    void setEdgeGlowAlpha(double v) {
        if (!qFuzzyCompare(m_edgeGlowAlpha, v)) { m_edgeGlowAlpha = v; m_styleDirty = true; emit styleChanged(); update(); }
    }

    double edgeGlowWidthScale() const { return m_edgeGlowWidthScale; }
    void setEdgeGlowWidthScale(double v) {
        if (!qFuzzyCompare(m_edgeGlowWidthScale, v)) { m_edgeGlowWidthScale = v; m_styleDirty = true; emit styleChanged(); update(); }
    }

    QColor virtualEdgeColor() const { return m_virtualEdgeColor; }
    void setVirtualEdgeColor(const QColor& c) {
        if (m_virtualEdgeColor != c) { m_virtualEdgeColor = c; m_styleDirty = true; emit styleChanged(); update(); }
    }

    QColor nodeCoreColor() const { return m_nodeCoreColor; }
    void setNodeCoreColor(const QColor& c) {
        if (m_nodeCoreColor != c) { m_nodeCoreColor = c; m_styleDirty = true; emit styleChanged(); update(); }
    }

    QColor nodeGlowColor() const { return m_nodeGlowColor; }
    void setNodeGlowColor(const QColor& c) {
        if (m_nodeGlowColor != c) { m_nodeGlowColor = c; m_styleDirty = true; emit styleChanged(); update(); }
    }

    double nodeCoreAlpha() const { return m_nodeCoreAlpha; }
    void setNodeCoreAlpha(double v) {
        if (!qFuzzyCompare(m_nodeCoreAlpha, v)) { m_nodeCoreAlpha = v; m_styleDirty = true; emit styleChanged(); update(); }
    }

    double nodeGlowAlpha() const { return m_nodeGlowAlpha; }
    void setNodeGlowAlpha(double v) {
        if (!qFuzzyCompare(m_nodeGlowAlpha, v)) { m_nodeGlowAlpha = v; m_styleDirty = true; emit styleChanged(); update(); }
    }

    double nodeGlowSizeScale() const { return m_nodeGlowSizeScale; }
    void setNodeGlowSizeScale(double v) {
        if (!qFuzzyCompare(m_nodeGlowSizeScale, v)) { m_nodeGlowSizeScale = v; m_styleDirty = true; emit styleChanged(); update(); }
    }

signals:
    void graphChanged();
    void zoomChanged();
    void offsetChanged();
    void lodIntensityChanged();
    void lodAutoZoomChanged();
    void hoveredEdgeChanged();
    void styleChanged();
    void momentumActiveChanged();
    void selectionChanged();
    void selectionInfoReady(QVariantMap info);
    void routeChanged();
    void routeModeChanged();
    void edgeViewModeChanged();
    void pathResultReady(QVariantMap info);
    void rangeHighlightChanged();

protected:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) override;

private:
    Graph* m_graph = nullptr;
    double m_zoom = 1.0;
    double m_lodIntensity = 1.0;
    bool   m_lodAutoZoom  = true;
    QPointF m_offset = QPointF(0, 0);

    int m_hoveredEdgeSource = -1;
    int m_hoveredEdgeTarget = -1;
    
    int m_edgeViewMode = 0; // 0: traffic, 1: centrality, 2: original

    QColor m_edgeCoolColor = QColor(0, 255, 255);
    QColor m_edgeMidColor = QColor(127, 0, 255);
    QColor m_edgeWarmColor = QColor(255, 0, 255);
    QColor m_edgeHoverColor = QColor(242, 255, 255);
    double m_edgeMidRatio = 0.55;
    double m_edgeCoreAlpha = 0.95;
    double m_edgeGlowAlpha = 0.32;
    double m_edgeGlowWidthScale = 1.9;
    QColor m_virtualEdgeColor = QColor(128, 128, 128);

    QColor m_nodeCoreColor = QColor(20, 30, 46);
    QColor m_nodeGlowColor = QColor(25, 51, 76);
    double m_nodeCoreAlpha = 0.98;
    double m_nodeGlowAlpha = 0.5;
    double m_nodeGlowSizeScale = 2.0;

public:
    Q_INVOKABLE QPointF mapToScreen(double x, double y) const;
    Q_INVOKABLE QPointF mapFromScreen(double sx, double sy) const;
private:
    QSGTexture *m_nodeTexture = nullptr;
    QSGTexture *m_rangeNodeTexture = nullptr;
    struct LogicalEdge {
        int u;
        int v;
        double ratio;
        int capacity;
        bool isDirect;
    };
    std::vector<LogicalEdge> m_cachedLogicalEdges;
    int m_cachedVisibleCount = -1;
    double m_bakedZoom = -1.0;
    double m_bakedLodZoom = -1.0; // zoom where LOD was last recalculated
    double m_cachedWidth = -1.0;
    double m_cachedHeight = -1.0;
    int m_cachedHoverLo = -1;
    int m_cachedHoverHi = -1;
    bool m_topologyDirty = true;
    bool m_geometryDirty = true;
    bool m_styleDirty = true;
    bool m_hoverDirty = false;

    // Grid caching: skip rebuild when zoom/offset change is below threshold
    double m_cachedGridSpacing = -1.0;
    double m_cachedGridOriginX = 0.0;
    double m_cachedGridOriginY = 0.0;

    // Momentum zoom: velocity accumulated from wheel events, decayed each tick
    double m_zoomVelocity = 0.0;
    bool m_momentumActive = false;
    QPointF m_zoomAnchor = QPointF(-1, -1); // anchor for mouse-centered momentum zoom
    QTimer* m_momentumTimer = nullptr;
    qint64 m_lastMomentumMs = 0;

    QTimer* m_trafficTimer = nullptr;
    QFutureWatcher<void> m_trafficWatcher;
    std::atomic<bool> m_isTrafficSimulating{false};

private slots:
    void onMomentumTick();
    void onAnimTick();

private:
    // 缓存地图范围，避免每帧重复计算
    struct Range {
        double minX = 0, maxX = 1;
        double minY = 0, maxY = 1;
    } m_range;
    
    void updateRange();
    bool m_rangeDirty = true;

    // 按度数（连边数）降序排列的节点索引列表，第一个元素就是全图最重要的交叉路口
    // 用于 LOD：缩小时只显示度数大的前 N 个节点
    std::vector<int> m_nodesByDegree; // 元素是 nodes[] 中的下标
    bool m_degreeDirty = true;
    void updateDegreeOrder();

    // Selection state
    int m_selectedNodeId = -1;
    int m_selectedEdgeSource = -1;
    int m_selectedEdgeTarget = -1;
    bool m_selectionDirty = false;
    QColor m_selectionAccent = QColor(59, 130, 246);

    // Route mode state
    bool m_routeMode = false;
    int m_routeStartNodeId = -1;
    int m_routeEndNodeId = -1;
    PathResult m_pathResult;
    bool m_pathDirty = false;
    QColor m_pathHighlightColor = QColor(251, 191, 36);

    // Edge-click endpoint highlight
    std::unordered_set<int> m_highlightedNodeIds;
    double m_nodeHighlightProgress = 0.0;
    QTimer* m_animTimer = nullptr;
    qint64 m_animStartMs = 0;

    // Range highlight state (coordinate-based nearest-N search)
    std::unordered_set<int> m_rangeHighlightNodeIds;
    std::unordered_set<uint64_t> m_rangeHighlightEdgeKeys;
    bool m_rangeHighlightDirty = false;

    // Overlay child tracking
    int m_baseChildCount = 0;

    void rebuildSelectionOverlay(QSGTransformNode* mapNode, float halfSize, double pixelsPerUnit);
};
