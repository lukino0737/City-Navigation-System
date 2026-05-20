#include "TrafficSimulator.h"
#include "../../api/NavigationAPI.h"
#include "../DataModel/Graph.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <unordered_map>
#include <vector>

TrafficSimulator& TrafficSimulator::instance() {
    static TrafficSimulator sim;
    return sim;
}

double TrafficSimulator::smoothNoise(double x, double y) {
    double v = 0.0;
    v += std::sin(x * 1.0 + m_simTime * 1.2);
    v += std::sin(y * 1.2 - m_simTime * 0.8);
    v += std::sin((x + y) * 0.8 + m_simTime * 1.5);
    v += std::sin((x - y) * 1.1 - m_simTime * 1.1);
    return v / 4.0;
}

void TrafficSimulator::ensureTopologyCache(const std::vector<Edge>& edges) {
    m_nodeToOutEdges.clear();
    m_edgeIdToIdx.clear();
    for (size_t i = 0; i < edges.size(); ++i) {
        m_nodeToOutEdges[edges[i].source].push_back(edges[i].id);
        m_edgeIdToIdx[edges[i].id] = static_cast<int>(i);
    }
    m_topologyCached = true;
}

void TrafficSimulator::invalidateTopologyCache() {
    m_topologyCached = false;
}

void TrafficSimulator::updateTrafficStatus(Graph& graph) {
    m_simTime += TIDAL_TIME_STEP;

    // Use shared lock to allow concurrent reads (like rendering)
    std::shared_lock<std::shared_mutex> dataLock(graph.dataMutex());
    const auto& edges = graph.getAllEdges();
    const auto& nodes = graph.getAllNodes();

    if (!m_topologyCached || m_edgeIdToIdx.size() != edges.size()) {
        // Drop shared lock and take exclusive lock if we need to rebuild cache
        dataLock.unlock();
        std::lock_guard<std::recursive_mutex> lock(graph.mutex());
        ensureTopologyCache(edges);
        dataLock.lock();
    }

    // Build a fast node-id to position map for the noise function
    // (We only need nodes involved in edges, but 10k nodes is fast to map)
    std::unordered_map<int, std::pair<double, double>> nodeCoords;
    nodeCoords.reserve(nodes.size());
    for (const auto& n : nodes) {
        nodeCoords[n.Node_id] = {n.x, n.y};
    }

    std::vector<int> delta_cars(edges.size(), 0);

    for (size_t i = 0; i < edges.size(); ++i) {
        const auto& edge = edges[i];
        if (edge.capacity <= 0) continue;

        double density = static_cast<double>(edge.currentCars) / edge.capacity;
        double current_speed = MAX_SPEED_COEFF * (1.0 - density);
        if (current_speed < 0) current_speed = 0;

        int expected_outflow = static_cast<int>(edge.currentCars * current_speed);

        auto it = m_nodeToOutEdges.find(edge.target);
        if (it == m_nodeToOutEdges.end() || it->second.empty()) {
            delta_cars[i] -= expected_outflow;
            continue;
        }

        const auto& downstream_edge_ids = it->second;
        int actual_outflow = 0;
        int flow_per_branch = expected_outflow / static_cast<int>(downstream_edge_ids.size());

        for (int down_id : downstream_edge_ids) {
            auto idx_it = m_edgeIdToIdx.find(down_id);
            if (idx_it == m_edgeIdToIdx.end()) continue;

            int down_idx = idx_it->second;
            const auto& down_edge = edges[down_idx];

            int available_space = down_edge.capacity - down_edge.currentCars - delta_cars[down_idx];
            int transferred_cars = std::min(flow_per_branch, std::max(0, available_space));

            delta_cars[down_idx] += transferred_cars;
            actual_outflow += transferred_cars;
        }

        delta_cars[i] -= actual_outflow;

        auto nIt = nodeCoords.find(edge.source);
        if (nIt != nodeCoords.end()) {
            double noiseVal = smoothNoise(nIt->second.first * NOISE_SPATIAL_SCALE, nIt->second.second * NOISE_SPATIAL_SCALE);
            if (noiseVal > TIDAL_NOISE_THRESHOLD && density < TIDAL_MAX_DENSITY) {
                delta_cars[i] += static_cast<int>(edge.capacity * TIDAL_INJECTION_RATE);
            }
        }
    }

    std::vector<int> updateIds;
    std::vector<int> updateCars;
    updateIds.reserve(edges.size());
    updateCars.reserve(edges.size());

    for (size_t i = 0; i < edges.size(); ++i) {
        int newCars = edges[i].currentCars + delta_cars[i];
        newCars = std::max(0, std::min(newCars, static_cast<int>(edges[i].capacity)));
        updateIds.push_back(edges[i].id);
        updateCars.push_back(newCars);
    }
    
    dataLock.unlock(); // Release read lock before applying updates
    graph.batchUpdateTraffic(updateIds, updateCars);
}

double TrafficSimulator::getEdgeTrafficWeight(const Edge& edge) const {
    if (edge.capacity <= 0) return DISCONNECTED_WEIGHT;

    double x = static_cast<double>(edge.currentCars) / edge.capacity;
    double f_x = 1.0;
    if (x > CONGESTION_THRESHOLD) {
        f_x = 1.0 + std::exp(x);
    }
    
    // T = c * L * f(x)
    return TRAFFIC_SPEED_COEFF * edge.length * f_x;
}

// 保持向后兼容的自由函数，委托给 TrafficSimulator 单例
void updateTrafficStatus(Graph& graph) {
    TrafficSimulator::instance().updateTrafficStatus(graph);
}

double getEdgeTrafficWeight(const Edge& edge) {
    return TrafficSimulator::instance().getEdgeTrafficWeight(edge);
}
