#include "algorithm.h"
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

    // 定义小顶堆：按花费(cost)从小到大排序
    std::priority_queue<State, std::vector<State>, std::greater<State>> pq;

    const int MAX_NODES = 10001; 

    std::vector<double> dist(MAX_NODES, std::numeric_limits<double>::infinity());
    std::vector<int> parent(MAX_NODES, -1);
    std::vector<bool> visited(MAX_NODES, false);
    
    // 初始化起点
    dist[startId] = 0.0;
    pq.push({0.0, startId});

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
            
            if (v <= 0 || v >= MAX_NODES) continue; 
            
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
        while (curr != startId) {
            result.path_nodes.push_back(curr);
            curr = parent[curr]; 
        }
        result.path_nodes.push_back(startId);
        // 因为是从终点往回找的，进行数组翻转
        std::reverse(result.path_nodes.begin(), result.path_nodes.end());
    }

    // 结算耗时
    auto end_time = std::chrono::high_resolution_clock::now();
    result.time_spent_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();

    return result;
}