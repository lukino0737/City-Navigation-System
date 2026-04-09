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


inline void to_json(nlohmann::json& j, const Node& p) {
    j = nlohmann::json{{"id", p.Node_id}, {"x", p.x}, {"y", p.y}};
}

inline void from_json(const nlohmann::json& j, Node& p) {
    j.at("id").get_to(p.Node_id);
    j.at("x").get_to(p.x);
    j.at("y").get_to(p.y);
}

struct PathResult {
    std::vector<int> pathNodes; // 途径的节点ID序列
    double totalCost;           // 总花费（距离或时间）
};

// ==========================================
// 成员 C 提供的接口 (底层数据加载)
// ==========================================
// 生成地图并存盘
bool generateAndSaveMap(int nodeCount, std::string filePath);
// 读取地图文件到内存 (更新为 2 个参数)
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
