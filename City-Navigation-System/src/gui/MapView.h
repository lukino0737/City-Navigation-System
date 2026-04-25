#pragma once

#include <QQuickItem>
#include <QSGNode>
#include <QSGGeometryNode>
#include <QSGFlatColorMaterial>
#include <QSGVertexColorMaterial>
#include <QVariant>
#include <QVariantMap>
#include <QPointF>
#include "../core/DataModel/Graph.h"

class MapView : public QQuickItem {
    Q_OBJECT
    Q_PROPERTY(Graph* graph READ graph WRITE setGraph NOTIFY graphChanged)
    // 缩放和平移属性，供 QML 交互使用
    Q_PROPERTY(double zoom READ zoom WRITE setZoom NOTIFY zoomChanged)
    Q_PROPERTY(QPointF offset READ offset WRITE setOffset NOTIFY offsetChanged)
    // LOD 开关：true = 随缩放动态调整节点数；false = 全量显示
    Q_PROPERTY(bool lodEnabled READ lodEnabled WRITE setLodEnabled NOTIFY lodEnabledChanged)

    Q_PROPERTY(int hoveredEdgeSource READ hoveredEdgeSource WRITE setHoveredEdgeSource NOTIFY hoveredEdgeChanged)
    Q_PROPERTY(int hoveredEdgeTarget READ hoveredEdgeTarget WRITE setHoveredEdgeTarget NOTIFY hoveredEdgeChanged)

public:
    explicit MapView(QQuickItem *parent = nullptr);
    ~MapView() override = default;

    Graph* graph() const { return m_graph; }
    void setGraph(Graph* graph);
    Q_INVOKABLE void refresh();

    Q_INVOKABLE QVariantMap hitTestNode(const QPointF& screenPos, double tolerance = 5.0) const;
    Q_INVOKABLE QVariantMap hitTestEdge(const QPointF& screenPos, double tolerance = 5.0) const;

    double zoom() const { return m_zoom; }
    void setZoom(double z);

    QPointF offset() const { return m_offset; }
    void setOffset(const QPointF& o);

    bool lodEnabled() const { return m_lodEnabled; }
    void setLodEnabled(bool v) {
        if (m_lodEnabled != v) { m_lodEnabled = v; emit lodEnabledChanged(); update(); }
    }

    int hoveredEdgeSource() const { return m_hoveredEdgeSource; }
    void setHoveredEdgeSource(int src) {
        if (m_hoveredEdgeSource != src) { m_hoveredEdgeSource = src; emit hoveredEdgeChanged(); update(); }
    }

    int hoveredEdgeTarget() const { return m_hoveredEdgeTarget; }
    void setHoveredEdgeTarget(int target) {
        if (m_hoveredEdgeTarget != target) { m_hoveredEdgeTarget = target; emit hoveredEdgeChanged(); update(); }
    }

signals:
    void graphChanged();
    void zoomChanged();
    void offsetChanged();
    void lodEnabledChanged();
    void hoveredEdgeChanged();

protected:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) override;

private:
    Graph* m_graph = nullptr;
    double m_zoom = 1.0;
    bool   m_lodEnabled = true; // 默认开启 LOD
    QPointF m_offset = QPointF(0, 0);

    int m_hoveredEdgeSource = -1;
    int m_hoveredEdgeTarget = -1;

public:
    Q_INVOKABLE QPointF mapToScreen(double x, double y) const;
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
};
