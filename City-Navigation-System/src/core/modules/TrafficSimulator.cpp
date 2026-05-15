#include "TrafficSimulator.h"
#include "../../api/NavigationAPI.h"
#include "../DataModel/Graph.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <unordered_map>
#include <vector>

TrafficSimulator::TrafficSimulator() {}
TrafficSimulator::~TrafficSimulator() {}

// 依然保留平滑噪声，但不再用于强行覆盖车流，而是用作模拟城市的“潮汐交通源”（如早晚高峰）
static double smoothNoise(double x, double y, double t) {
    double v = 0.0;
    v += std::sin(x * 1.0 + t * 1.2);
    v += std::sin(y * 1.2 - t * 0.8);
    v += std::sin((x + y) * 0.8 + t * 1.5);
    v += std::sin((x - y) * 1.1 - t * 1.1);
    return v / 4.0;
}

void updateTrafficStatus(Graph &graph) {
    static double time_t = 0.0;
    time_t += 0.02; // 控制潮汐变化的速率

    std::lock_guard<std::recursive_mutex> lock(graph.mutex());
    auto &edges = const_cast<std::vector<Edge> &>(graph.getAllEdges());

    // 【优化1】静态缓存路网拓扑，极大提升 10,000+ 节点下的遍历与转移性能
    static std::unordered_map<int, std::vector<int>> node_to_out_edges;
    static std::unordered_map<int, int> edge_id_to_idx;
    static bool is_topology_cached = false;

    if (!is_topology_cached) {
        for (size_t i = 0; i < edges.size(); ++i) {
            node_to_out_edges[edges[i].source].push_back(edges[i].id);
            edge_id_to_idx[edges[i].id] = i;
        }
        is_topology_cached = true;
    }

    // 记录每个路段在此时间步的车辆净变化量
    std::vector<int> delta_cars(edges.size(), 0);

    // 【优化2】基于宏观交通流 (Macroscopic Traffic Flow) 模拟车辆的真实物理转移
    for (size_t i = 0; i < edges.size(); ++i) {
        auto &edge = edges[i];
        if (edge.capacity <= 0) continue;

        // 交通流密度 (Density) = 当前车辆 / 容量
        double density = static_cast<double>(edge.currentCars) / edge.capacity;

        // 采用 Greenshields 速度-密度线性关系模型
        // 车越密集，实际通行速度越慢。基础自由流速度系数设为 0.2
        double current_speed = 0.2 * (1.0 - density);
        if (current_speed < 0) current_speed = 0;

        // 期望流出量 (Flow) = 密度 * 速度 * 容量
        int expected_outflow = static_cast<int>(edge.currentCars * current_speed);

        // 如果到达死胡同或终点节点，车辆自然消散离开路网
        if (node_to_out_edges.find(edge.target) == node_to_out_edges.end() || node_to_out_edges[edge.target].empty()) {
            delta_cars[i] -= expected_outflow;
            continue;
        }

        // 尝试将车流转移到下游路段
        const auto& downstream_edge_ids = node_to_out_edges[edge.target];
        int actual_outflow = 0;
        int flow_per_branch = expected_outflow / downstream_edge_ids.size();

        // 【优化3】拥堵反向传播 (Backward Shockwave) 机制
        for (int down_id : downstream_edge_ids) {
            int down_idx = edge_id_to_idx[down_id];
            auto &down_edge = edges[down_idx];
            
            // 下游路段必须有空间才能进车。如果下游堵死，上游的车就过不去
            int available_space = down_edge.capacity - down_edge.currentCars - delta_cars[down_idx];
            int transferred_cars = std::min(flow_per_branch, std::max(0, available_space));

            delta_cars[down_idx] += transferred_cars;
            actual_outflow += transferred_cars;
        }

        // 本路段扣除实际成功流出的车辆
        // 如果下游全堵死，actual_outflow 为 0，本路段车辆不减少，从而实现拥堵向后蔓延
        delta_cars[i] -= actual_outflow;

        // 【优化4】动态外部流量注入（潮汐车流生成）
        Node n1 = graph.getNode(edge.source);
        double noiseVal = smoothNoise(n1.x * 0.002, n1.y * 0.002, time_t);
        
        // 模拟特定时间段、特定区域（如早高峰的住宅区）持续涌入的外部车流
        if (noiseVal > 0.65 && density < 0.9) {
            delta_cars[i] += static_cast<int>(edge.capacity * 0.05);
        }
    }

    // 统一结算并应用本轮所有的车流变化，确保数据一致性
    for (size_t i = 0; i < edges.size(); ++i) {
        int newCars = edges[i].currentCars + delta_cars[i];
        // 严格保证车辆数在 [0, capacity] 之间
        newCars = std::max(0.0, std::min((double)newCars, edges[i].capacity));
        graph.updateEdgeTraffic(edges[i].id, newCars);
    }
}

double getEdgeTrafficWeight(const Edge &edge) {
    if (edge.capacity <= 0) return 999999.0;

    double v_c_ratio = static_cast<double>(edge.currentCars) / edge.capacity;

    // 【优化5】采用交通工程界公认的标准：BPR (Bureau of Public Roads) 阻抗函数
    // 公式: T = T_free * [1 + α * (V/C)^β]
    const double alpha = 0.15;
    const double beta = 4.0; 

    // 基础自由流通行时间 (T_free)，保持原代码的 c=0.1 基础常数
    double free_flow_time = 0.1 * edge.length; 

    // 拥堵时间计算
    // 当 V/C (饱和度) 接近或超过 1.0 时，通行时间会呈四次方指数级飙升，极度契合真实驾驶体验
    double actual_time = free_flow_time * (1.0 + alpha * std::pow(v_c_ratio, beta));

    return actual_time;
}
