#include "DataGenerator.h"
#include "../DataModel/Graph.h"
#include "../../api/NavigationAPI.h"
#include <cmath>
#include <algorithm>
#include <vector>

DataGenerator::DataGenerator() {}
DataGenerator::~DataGenerator() {}

// 计算两点欧式距离
double calculateDistance(const Node& a, const Node& b) {
    return std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2));
}

bool generateAndSaveMap(int nodeCount, std::string filePath) {
    Graph graph;
    
    // 1. 随机生成节点
    for (int i = 0; i < nodeCount; i++) {
        int x = rand() % 1000;
        int y = rand() % 1000;
        graph.addNode(i, x, y);
    }

    const auto& nodes = graph.getAllNodes();
    int edgeIdCounter = 0;

    // 2. 为每个节点连接到最近的 k 个邻居 (例如 k=3)
    int k = 3;
    for (int i = 0; i < nodeCount; i++) {
        std::vector<std::pair<double, int>> neighbors;
        for (int j = 0; j < nodeCount; j++) {
            if (i == j) continue;
            double dist = calculateDistance(nodes[i], nodes[j]);
            neighbors.push_back({dist, j});
        }
        
        // 按距离排序
        std::sort(neighbors.begin(), neighbors.end());

        // 添加前 k 个邻居作为边
        for (int m = 0; m < k && m < neighbors.size(); m++) {
            double dist = neighbors[m].first;
            int targetId = neighbors[m].second;
            // 为简化，这里直接添加有向边，实际可根据需要改为无向
            graph.addEdge(edgeIdCounter++, i, targetId, dist, 100.0 + (rand() % 400));
        }
    }

    // 3. 保存到 JSON 文件
    return graph.save(filePath);
}

bool loadMap(Graph & graph,std::string filePath) {
    return graph.load(filePath);
}
