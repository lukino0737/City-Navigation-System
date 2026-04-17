#pragma once
#include <vector>
#include <string>
#include "../../3rdparty/nlohmann/json.hpp"

// 前置声明
class Graph;

// 基础数据结构定义
struct Node {
    int Node_id;
    int x;
    int y;
};

struct Edge {
    int id;
    int source;     // 起点 ID
    int target;     // 终点 ID
    double length;
    double capacity; 
    int currentCars; 
};

// 寻路结果包
struct PathResult {
    bool success;                   // 是否成功找到路径
    std::vector<int> path_nodes;    // 途径的节点ID序列
    double total_cost;              // 总花费（距离或时间）

    // 用来在界面上做动画的数据
    double time_spent_ms;           // 算法纯计算耗时(毫秒)
    std::vector<int> explored_nodes;// 算法探索过的节点顺序（像水波一样渲染）
};

// ==========================================
// 序列化逻辑：用于将Node和Edge对象转换为JSON格式以实现存储
// ==========================================
namespace nlohmann {
    template <>
    struct adl_serializer<Node> {
        static void to_json(json& j, const Node& p) {
            j = json{{"id", p.Node_id}, {"x", p.x}, {"y", p.y}};
        }
        static void from_json(const json& j, Node& p) {
            j.at("id").get_to(p.Node_id);
            j.at("x").get_to(p.x);
            j.at("y").get_to(p.y);
        }
    };

    template <>
    struct adl_serializer<Edge> {
        static void to_json(json& j, const Edge& e) {
            j = json{{"id", e.id}, {"source", e.source}, {"target", e.target}, 
                     {"length", e.length}, {"capacity", e.capacity}, {"currentCars", e.currentCars}};
        }
        static void from_json(const json& j, Edge& e) {
            j.at("id").get_to(e.id);
            j.at("source").get_to(e.source);
            j.at("target").get_to(e.target);
            j.at("length").get_to(e.length);
            j.at("capacity").get_to(e.capacity);
            j.at("currentCars").get_to(e.currentCars);
        }
    };
}

// ==========================================
// 成员 C 提供的接口 (底层数据加载)
// ==========================================

// 获取Graph对象中的所有节点和边集合
std::vector<Node> getNodes(Graph& graph);
std::vector<Edge> getEdges(Graph& graph);

// 获取某节点所有邻边
std::vector<Edge> getEdgesFromNode(Graph& graph, int nodeId);


// 生成地图并存盘
bool generateAndSaveMap(int nodeCount, std::string filePath);
// 读取地图文件到内存
bool loadMap(Graph& graph, std::string filePath); 

// ==========================================
// 成员 B 提供的接口 (空间搜索与动态车流)
// ==========================================
std::vector<Node> getNearestPoints(double x, double y, int count);
void updateTrafficStatus();
double getEdgeTrafficWeight(int edgeId);

// ==========================================
// 成员 A 提供的接口 (核心寻路引擎)
// ==========================================
PathResult calculateShortestPath(int startId, int endId, bool considerTraffic);
