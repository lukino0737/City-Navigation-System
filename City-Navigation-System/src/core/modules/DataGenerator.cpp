#include "DataGenerator.h"
#include "../../api/NavigationAPI.h"
#include "../DataModel/Graph.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>
#include <vector>

// 为了加速内层循环的比对，避免调用极慢的开根号 sqrt
inline double calculateDistanceSq(const Node &a, const Node &b) {
  return std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2);
}

// 需要实际距离数值时的标准公式
double calculateDistance(const Node &a, const Node &b) {
  return std::sqrt(calculateDistanceSq(a, b));
}

// 检查两点是否已有通道，防止双向重线堆叠
bool hasEdge(Graph &graph, int u, int v) {
  for (const auto &e : graph.getEdgesFrom(u)) {
    if (e.target == v)
      return true;
  }
  return false;
}

bool generateAndSaveMap(int nodeCount, std::string filePath) {
  Graph graph;

  // 我们约定的扩大尺寸，避免人挤人
  const double MAP_SIZE = 10000.0;

  std::random_device rd;
  std::mt19937 gen(rd());

  int numCenters = 5;
  std::vector<std::pair<double, double>> centers;
  std::uniform_real_distribution<> disCenter(MAP_SIZE * 0.1, MAP_SIZE * 0.9);
  for (int i = 0; i < numCenters; i++) {
    centers.push_back({disCenter(gen), disCenter(gen)});
  }

  std::uniform_real_distribution<> disUniform(0, MAP_SIZE);

  // 【1】铺排全市路口地基
  for (int i = 0; i < nodeCount; i++) {
    double x, y;
    if (i < nodeCount * 0.6) { // 核心密集的市中心居民区
      int centerIdx = i % numCenters;
      std::normal_distribution<> disNormX(centers[centerIdx].first,
                                          MAP_SIZE * 0.08);
      std::normal_distribution<> disNormY(centers[centerIdx].second,
                                          MAP_SIZE * 0.08);
      x = std::clamp(disNormX(gen), 0.0, MAP_SIZE - 1.0);
      y = std::clamp(disNormY(gen), 0.0, MAP_SIZE - 1.0);
    } else { // 均匀分布的偏荒近郊
      x = disUniform(gen);
      y = disUniform(gen);
    }
    graph.addNode(i, static_cast<int>(x), static_cast<int>(y));
  }

  const auto &nodes = graph.getAllNodes();
  int edgeIdCounter = 0;

  std::uniform_real_distribution<> disCapLocal(50.0, 150.0);
  std::uniform_real_distribution<> disCapArterial(300.0, 600.0);

  std::cout << "\nDataGenerator: 正在使用 Prim 算法构建包含 " << nodeCount
            << " 个点的全国主干快线骨架..." << std::endl;

  // 【2】Prim最小生成树
  std::vector<bool> inMST(nodeCount, false);
  std::vector<double> minWeight(nodeCount, 2e18); // 到现有树林的最小平方距
  std::vector<int> parent(nodeCount, -1);

  minWeight[0] = 0.0;

  for (int count = 0; count < nodeCount; ++count) {
    int u = -1;
    double min = 2e18;

    // 步骤 a：全图搜寻能最廉价并入现有文明版图的野节点
    for (int v = 0; v < nodeCount; ++v) {
      if (!inMST[v] && minWeight[v] < min) {
        min = minWeight[v];
        u = v;
      }
    }

    if (u == -1)
      break; // 若无法继续（防崩溃兜底）

    inMST[u] = true;

    // 步骤 b：把这个离树干最近的点真正用双主路接合进公路网中
    if (parent[u] != -1) {
      double dist = calculateDistance(nodes[u], nodes[parent[u]]);
      graph.addEdge(edgeIdCounter++, u, parent[u], dist, disCapArterial(gen));
      graph.addEdge(edgeIdCounter++, parent[u], u, dist, disCapArterial(gen));
    }

    // 步骤
    // c：由于这名新成员的加入，刷新一下所有还流落在外的孤点与树干的最短联系成本
    for (int v = 0; v < nodeCount; ++v) {
      if (!inMST[v]) {
        double distSq = calculateDistanceSq(nodes[u], nodes[v]);
        if (distSq < minWeight[v]) {
          parent[v] = u;
          minWeight[v] = distSq;
        }
      }
    }
  }

  std::cout << "DataGenerator: "
               "最小生成树(MST)骨干结界架构完成！已实现完全连通绝无任何死点！"
            << std::endl;

  // 【3】城市肌理添加：在绝对连通树上铺排网格化的细小辅道路，充当环形路线避免绕巨远的错路
  const double CELL_SIZE = 400.0;
  int gridSize = std::ceil(MAP_SIZE / CELL_SIZE);
  std::vector<std::vector<int>> grid(gridSize * gridSize);

  for (int i = 0; i < nodeCount; ++i) {
    int gx =
        std::clamp(static_cast<int>(nodes[i].x / CELL_SIZE), 0, gridSize - 1);
    int gy =
        std::clamp(static_cast<int>(nodes[i].y / CELL_SIZE), 0, gridSize - 1);
    grid[gy * gridSize + gx].push_back(i);
  }

  std::cout << "DataGenerator: 正在穿插周边辅带街道路网..." << std::endl;

  int k_redundancy = 2; // 只需为树骨架追加寥寥分支作闭环即可
  for (int i = 0; i < nodeCount; ++i) {
    std::vector<std::pair<double, int>> neighbors;
    int gx = static_cast<int>(nodes[i].x / CELL_SIZE);
    int gy = static_cast<int>(nodes[i].y / CELL_SIZE);

    int searchRadius = 1;
    while (neighbors.size() < k_redundancy + 1 && searchRadius < 7) {
      neighbors.clear();
      for (int dy = -searchRadius; dy <= searchRadius; ++dy) {
        for (int dx = -searchRadius; dx <= searchRadius; ++dx) {
          int nx = gx + dx;
          int ny = gy + dy;
          if (nx >= 0 && nx < gridSize && ny >= 0 && ny < gridSize) {
            for (int nIdx : grid[ny * gridSize + nx]) {
              if (nIdx != i) {
                double dist = calculateDistance(nodes[i], nodes[nIdx]);
                neighbors.push_back({dist, nIdx});
              }
            }
          }
        }
      }
      if (neighbors.size() >= k_redundancy)
        break;
      searchRadius++;
    }

    std::sort(neighbors.begin(), neighbors.end());

    int addedRedundant = 0;
    int max_check = std::min((int)neighbors.size(), 8);
    for (int m = 0; m < max_check && addedRedundant < k_redundancy; ++m) {
      double dist = neighbors[m].first;
      int targetId = neighbors[m].second;

      bool added = false;
      if (!hasEdge(graph, i, targetId)) {
        graph.addEdge(edgeIdCounter++, i, targetId, dist, disCapLocal(gen));
        added = true;
      }
      if (!hasEdge(graph, targetId, i)) {
        graph.addEdge(edgeIdCounter++, targetId, i, dist, disCapLocal(gen));
        added = true;
      }
      if (added)
        addedRedundant++;
    }
  }

  std::cout << "DataGenerator: 所有普通城内街道分发完毕。共搭载 " << nodeCount
            << " 节点与 " << edgeIdCounter << " 条双向通道。开始生成 Json ..."
            << std::endl;

  // 【4】持久化存储
  return graph.save(filePath);
}

bool loadMap(Graph &graph, std::string filePath) {
  return graph.load(filePath);
}
