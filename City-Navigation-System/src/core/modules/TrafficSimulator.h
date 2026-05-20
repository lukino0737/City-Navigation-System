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

    // 现实化路况模型参数
    static constexpr double TRAFFIC_SPEED_COEFF = 0.015; // c: 约 40km/h 对应的 min/unit
    static constexpr double CONGESTION_THRESHOLD = 0.8;  // x 临界值
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
