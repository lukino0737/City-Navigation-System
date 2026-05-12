#include "algorithm.h"
#include "../DataModel/Graph.h"
#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <limits>
#include <chrono> 
#include <algorithm>

// 定义优先队列里的元素：{当前走过的总花费, 节点ID}
using State = std::pair<double, int>;

PathResult calculateShortestPath(int startId, int endId, bool considerTraffic,Graph& graph) {
    PathResult result;
    result.success = false;
    result.total_cost = 0.0;
    
    // 记录算法开始时间
    auto start_time = std::chrono::high_resolution_clock::now();

    // 动态获取最大节点 ID
    int maxNodeId = 0;
    for (const auto& n : graph.getAllNodes()) {
        if (n.Node_id > maxNodeId) {
            maxNodeId = n.Node_id;
        }
    }
    const int MAX_NODES = maxNodeId + 1;

    // 定义小顶堆：按花费(cost)从小到大排序
    std::priority_queue<State, std::vector<State>, std::greater<State>> pq;

    std::vector<double> dist(MAX_NODES, std::numeric_limits<double>::infinity());
    std::vector<int> parent(MAX_NODES, -1);
    std::vector<bool> visited(MAX_NODES, false);
    
    // 初始化起点
    if (startId >= 0 && startId < MAX_NODES) {
        dist[startId] = 0.0;
        pq.push({0.0, startId});
    }

    while (!pq.empty()) {
        State current = pq.top();
        pq.pop();
        
        double currentCost = current.first;
        int u = current.second;

        if (u < 0 || u >= MAX_NODES) continue; 

        if (visited[u]) continue;
        
        visited[u] = true;
        result.explored_nodes.push_back(u);

        if (u == endId) {
            result.success = true;
            result.total_cost = currentCost;
            break; 
        }

        std::vector<Edge> neighbors = getEdgesFromNode(graph, u);
        
        for (const Edge& edge : neighbors) {
            int v = edge.target;
            
            // 修复：之前这里是 v <= 0，导致跳过了节点0的路径
            if (v < 0 || v >= MAX_NODES) continue; 
            
            double edgeWeight = edge.length;
            if (considerTraffic) {
                edgeWeight = getEdgeTrafficWeight(edge.id); 
            }

            double newCost = currentCost + edgeWeight;
            
            if (newCost < dist[v]) {
                dist[v] = newCost;
                parent[v] = u; 
                pq.push({newCost, v});
            }
        }
    }

    // 路径回溯 (如果成功找到的话)
    if (result.success) {
        int curr = endId;
        while (curr != startId && curr != -1) {
            result.path_nodes.push_back(curr);
            curr = parent[curr]; 
        }
        if (curr == startId) {
            result.path_nodes.push_back(startId);
            // 因为是从终点往回找的，进行数组翻转
            std::reverse(result.path_nodes.begin(), result.path_nodes.end());
        } else {
            result.success = false;
            result.path_nodes.clear();
        }
    }

    // 结算耗时
    auto end_time = std::chrono::high_resolution_clock::now();
    result.time_spent_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();

    return result;
}
