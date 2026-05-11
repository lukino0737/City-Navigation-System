#include<iostream>
#include"../../api/NavigationAPI.h"
#include<vector>
#include<string>
#include<queue>
#include<unordered_map>
#include<limits>
#include<chrono> // 用来算 time_spent_ms

// 定义优先队列里的元素：{当前走过的总花费, 节点ID}
using State = std::pair<double, int>;

PathResult calculateShortestPath(int startId, int endId, bool considerTraffic,Graph& graph) {
    PathResult result;
    result.success = false;
    result.total_cost = 0.0;
    
    // 记录算法开始时间
    auto start_time = std::chrono::high_resolution_clock::now();

    // 1. 定义小顶堆：按花费(cost)从小到大排序
    std::priority_queue<State, std::vector<State>, std::greater<State>> pq;
    
   // 💡 架构师优化：给上限加 1！因为 ID 从 1 到 10000，所以我们需要下标能取到 10000
    const int MAX_NODES = 10001; 

    // 连续内存预分配：现在下标范围是 0 到 10000，完美覆盖！
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

        // 🛡️ 安全校验升级：有效的 ID 应该是 1 到 10000 (即 u > 0 且 u < MAX_NODES)
        if (u <= 0 || u >= MAX_NODES) continue; 

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
            
            // 🛡️ 邻居节点的防越界校验
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

    // 5. 路径回溯 (如果成功找到的话)
    if (result.success) {
        int curr = endId;
        while (curr != startId) {
            result.path_nodes.push_back(curr);
            curr = parent[curr]; // 一步步往回找爸爸
        }
        result.path_nodes.push_back(startId);
        // 因为是从终点往回找的，最后记得把数组翻转一下
        std::reverse(result.path_nodes.begin(), result.path_nodes.end());
    }

    // 6. 结算耗时
    auto end_time = std::chrono::high_resolution_clock::now();
    result.time_spent_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();

    return result;
}