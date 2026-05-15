#pragma once

#include <unordered_map>
#include <vector>

class Graph;
struct Edge;

class TrafficSimulator {
public:
    static TrafficSimulator& instance();

    void updateTrafficStatus(Graph& graph);
    double getEdgeTrafficWeight(const Edge& edge) const;
    void invalidateTopologyCache();

private:
    TrafficSimulator() = default;

    double smoothNoise(double x, double y);
    void ensureTopologyCache(const std::vector<Edge>& edges);

    // BPR (Bureau of Public Roads) 阻抗函数参数
    static constexpr double BPR_ALPHA = 0.15;
    static constexpr double BPR_BETA = 4.0;
    static constexpr double FREE_FLOW_TIME_COEFF = 0.1;

    // Greenshields 速度-密度线性关系模型参数
    static constexpr double MAX_SPEED_COEFF = 0.2;

    // 潮汐交通流参数
    static constexpr double TIDAL_TIME_STEP = 0.02;
    static constexpr double TIDAL_NOISE_THRESHOLD = 0.65;
    static constexpr double TIDAL_MAX_DENSITY = 0.9;
    static constexpr double TIDAL_INJECTION_RATE = 0.05;
    static constexpr double NOISE_SPATIAL_SCALE = 0.002;

    static constexpr double DISCONNECTED_WEIGHT = 999999.0;

    double m_simTime = 0.0;

    std::unordered_map<int, std::vector<int>> m_nodeToOutEdges;
    std::unordered_map<int, int> m_edgeIdToIdx;
    bool m_topologyCached = false;
};
