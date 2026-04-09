#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include "../../api/NavigationAPI.h"

using json = nlohmann::json;
// 道路边结构体
struct Edge {
    int id;
    int source;     // 起点 ID
    int target;     // 终点 ID
    double length;
    double capacity; 
    int currentCars; 
};


inline void to_json(nlohmann::json& j, const Edge& e) {
    j = nlohmann::json{{"id", e.id}, {"source", e.source}, {"target", e.target}, {"length", e.length}, {"capacity", e.capacity}, {"currentCars", e.currentCars}};
}

inline void from_json(const nlohmann::json& j, Edge& e) {
    j.at("id").get_to(e.id);
    j.at("source").get_to(e.source);
    j.at("target").get_to(e.target);
    j.at("length").get_to(e.length);
    j.at("capacity").get_to(e.capacity);
    j.at("currentCars").get_to(e.currentCars);
}

class Graph {
private:
    std::vector<Node> m_nodes;                     // 节点集合
    std::vector<Edge> m_edges;                     // 边集合
    std::unordered_map<int, std::vector<Edge>> m_adjList; // 邻接表
    std::unordered_map<int, size_t> m_nodeIdToIndex; // 映射
    std::unordered_map<int, size_t> m_edgeIdToIndex; // 映射

public:
    Graph();
    ~Graph();

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
