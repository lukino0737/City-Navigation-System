#pragma once

#include <QQuickItem>
#include <QSGNode>
#include <QSGGeometryNode>
#include <QSGFlatColorMaterial>
#include <QSGVertexColorMaterial>
#include "../core/DataModel/Graph.h"

class MapView : public QQuickItem {
    Q_OBJECT
    Q_PROPERTY(Graph* graph READ graph WRITE setGraph NOTIFY graphChanged)
    // 缩放和平移属性，供 QML 交互使用
    Q_PROPERTY(double zoom READ zoom WRITE setZoom NOTIFY zoomChanged)
    Q_PROPERTY(QPointF offset READ offset WRITE setOffset NOTIFY offsetChanged)
    // LOD 开关：true = 随缩放动态调整节点数；false = 全量显示
    Q_PROPERTY(bool lodEnabled READ lodEnabled WRITE setLodEnabled NOTIFY lodEnabledChanged)

public:
    explicit MapView(QQuickItem *parent = nullptr);
    ~MapView() override = default;

    Graph* graph() const { return m_graph; }
    void setGraph(Graph* graph);
    Q_INVOKABLE void refresh();

    double zoom() const { return m_zoom; }
    void setZoom(double z);

    QPointF offset() const { return m_offset; }
    void setOffset(const QPointF& o);

    bool lodEnabled() const { return m_lodEnabled; }
    void setLodEnabled(bool v) {
        if (m_lodEnabled != v) { m_lodEnabled = v; emit lodEnabledChanged(); update(); }
    }

signals:
    void graphChanged();
    void zoomChanged();
    void offsetChanged();
    void lodEnabledChanged();

protected:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) override;

private:
    Graph* m_graph = nullptr;
    double m_zoom = 1.0;
    bool   m_lodEnabled = true; // 默认开启 LOD
    QPointF m_offset = QPointF(0, 0);

    // 辅助函数：将地图坐标转换为 QML 屏幕坐标
    QPointF mapToScreen(double x, double y) const;
    
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
