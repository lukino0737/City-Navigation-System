#pragma once
#include <QObject>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include "../../api/NavigationAPI.h"

using json = nlohmann::json;

std::vector<Node> getNodes(Graph& graph);
std::vector<Edge> getEdges(Graph& graph);
class Graph : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(Graph)

private:
    std::vector<Node> m_nodes;                     // 节点集合
    std::vector<Edge> m_edges;                     // 边集合
    std::unordered_map<int, std::vector<Edge>> m_adjList; // 邻接表
    std::unordered_map<int, size_t> m_nodeIdToIndex; // 映射
    std::unordered_map<int, size_t> m_edgeIdToIndex; // 映射

public:
    explicit Graph(QObject *parent = nullptr) : QObject(parent) {}
    ~Graph() override = default;

    // 加载和保存地图
    bool load(std::string filePath);
    bool save(std::string path);
    
    // 数据操作接口
    Node getNode(int id);
    std::vector<Edge>& getEdgesFrom(int nodeId);
    Edge& getEdgeById(int edgeId);
    void updateEdgeTraffic(int edgeId, int carCount);

    const std::vector<Node>& getAllNodes();
    const std::vector<Edge>& getAllEdges();

    bool addNode(int id, int x, int y);
    bool addEdge(int id, int source, int target, double length, double capacity);
};
