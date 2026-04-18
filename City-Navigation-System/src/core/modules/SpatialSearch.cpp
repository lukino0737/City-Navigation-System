//#include "SpatialSearch.h"
//#include "../../api/NavigationAPI.h"
//#include <vector>

//SpatialSearch::SpatialSearch() {}
//SpatialSearch::~SpatialSearch() {}

//std::vector<Node> getNearestPoints(double x, double y, int count) {
    // TODO: 实现获取距离 (x,y) 最近的 count 个点
 //   return std::vector<Node>();
//}
#include "SpatialSearch.h"
#include <cmath>
#include <queue>
#include <algorithm>

SpatialSearch::SpatialSearch(Graph* graph) : m_graph(graph) {}

SpatialSearch::~SpatialSearch() {}

std::vector<Node> SpatialSearch::getNearestPoints(double x, double y, int count) {
    if (!m_graph || count <= 0) return {};

    const auto& nodes = m_graph->getAllNodes();
    
    // 定义一个计算两点欧氏距离平方的 Lambda 函数
    auto distSq = [](double x1, double y1, double x2, double y2) {
        return (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
    };

    // 使用最大堆来维护距离最近的 count 个节点
    // 堆里存储 pair: {距离的平方, 节点}
    using Pair = std::pair<double, Node>;
    auto cmp = [](const Pair& left, const Pair& right) {
        return left.first < right.first; // 最大堆
    };
    std::priority_queue<Pair, std::vector<Pair>, decltype(cmp)> maxHeap(cmp);

    for (const auto& node : nodes) {
        // 假设 NavigationAPI.h 中的 Node 包含 x, y 坐标属性
        double d2 = distSq(x, y, node.x, node.y); 
        
        if (maxHeap.size() < count) {
            maxHeap.push({d2, node});
        } else if (d2 < maxHeap.top().first) {
            maxHeap.pop();
            maxHeap.push({d2, node});
        }
    }

    // 将结果从堆中取出并按距离从小到大排序
    std::vector<Node> result;
    while (!maxHeap.empty()) {
        result.push_back(maxHeap.top().second);
        maxHeap.pop();
    }
    std::reverse(result.begin(), result.end());

    return result;
}
