#pragma once
#include <QObject>
#include <QStringList>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <mutex>
#include <shared_mutex>
#include "../../api/NavigationAPI.h"

using json = nlohmann::json;

class Graph : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(Graph)

private:
    mutable std::recursive_mutex m_mutex;
    mutable std::shared_mutex m_dataMutex;          // 保护 m_nodes/m_edges/m_adjList 渲染线程读取
    std::vector<Node> m_nodes;                     // 节点集合
    std::vector<Edge> m_edges;                     // 边集合
    std::unordered_map<int, std::vector<Edge>> m_adjList; // 邻接表
    std::unordered_map<int, size_t> m_nodeIdToIndex; // 映射
    std::unordered_map<int, size_t> m_edgeIdToIndex; // 映射

    // 地图文件管理
    QStringList m_availableMaps;
    QStringList m_availableMapFiles;
    int m_currentMapIndex = -1;

public:
    explicit Graph(QObject *parent = nullptr) : QObject(parent) {}
    ~Graph() override = default;

    std::recursive_mutex& mutex() const { return m_mutex; }
    std::shared_mutex& dataMutex() const { return m_dataMutex; }

    // 加载和保存地图
    bool load(std::string filePath);
    bool save(std::string path);
    Q_INVOKABLE void regenerateGraph(int nodeCount = 10000);
    Q_INVOKABLE void reloadSimulationMap();

    Q_PROPERTY(QStringList availableMaps READ availableMaps NOTIFY availableMapsChanged)
    Q_PROPERTY(int currentMapIndex READ currentMapIndex NOTIFY currentMapIndexChanged)
    Q_INVOKABLE void refreshAvailableMaps();
    Q_INVOKABLE void switchToMap(int index);
    QStringList availableMaps() const { return m_availableMaps; }
    int currentMapIndex() const { return m_currentMapIndex; }

    signals:
    void graphRegenerated();
    void availableMapsChanged();
    void currentMapIndexChanged();
    
public:
    // 数据操作接口
    Node getNode(int id);
    std::vector<Edge>& getEdgesFrom(int nodeId);
    Edge& getEdgeById(int edgeId);
    void updateEdgeTraffic(int edgeId, int carCount);

    const std::vector<Node>& getAllNodes();
    const std::vector<Edge>& getAllEdges();

    bool addNode(int id, double x, double y, const std::string& name = "");
    bool addEdge(int id, int source, int target, double length, double capacity, double centrality = 0.0);
};

// Global helper declarations
std::vector<Node> getNodes(Graph& graph);
std::vector<Edge> getEdges(Graph& graph);
std::vector<Edge> getEdgesFromNode(Graph& graph, int nodeId);
Node getNodeById(Graph& graph, int nodeId);
Edge getEdgeById(Graph& graph, int edgeId);
