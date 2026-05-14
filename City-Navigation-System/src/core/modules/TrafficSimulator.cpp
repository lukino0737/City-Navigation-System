#include "TrafficSimulator.h"
#include "../../api/NavigationAPI.h"
#include "../DataModel/Graph.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

TrafficSimulator::TrafficSimulator() {}
TrafficSimulator::~TrafficSimulator() {}

// A fast sum-of-sines approximation to create continuous "blobs" like Perlin
// noise
static double smoothNoise(double x, double y, double t) {
  double v = 0.0;
  v += std::sin(x * 1.0 + t * 1.2);
  v += std::sin(y * 1.2 - t * 0.8);
  v += std::sin((x + y) * 0.8 + t * 1.5);
  v += std::sin((x - y) * 1.1 - t * 1.1);
  return v / 4.0; // range roughly [-1.0, 1.0]
}

void updateTrafficStatus(Graph &graph) {
  static double time_t = 0.0;
  time_t += 0.05; // Time delta per update

  std::lock_guard<std::recursive_mutex> lock(graph.mutex());
  auto &edges = const_cast<std::vector<Edge> &>(graph.getAllEdges());

  for (auto &edge : edges) {
    Node n1 = graph.getNode(edge.source);
    Node n2 = graph.getNode(edge.target);

    // Midpoint of the edge
    double midX = (n1.x + n2.x) / 2.0;
    double midY = (n1.y + n2.y) / 2.0;

    // Scale coordinates for the noise function to control blob size
    double scale = 0.002;

    double noiseVal = smoothNoise(midX * scale, midY * scale, time_t);

    // Map noise to congestion ratio (0.2 ~ 1.5)
    double congestionRatio = 0.85 + noiseVal * 0.65;

    // Add tiny random jitter
    double randomJitter = (std::rand() % 200 - 100) / 2000.0; // -0.05 to +0.05
    congestionRatio += randomJitter;

    if (congestionRatio < 0.05)
      congestionRatio = 0.05;

    int newCars = static_cast<int>(edge.capacity * congestionRatio);
    graph.updateEdgeTraffic(edge.id, newCars);
  }
}

double getEdgeTrafficWeight(const Edge &edge) {
  if (edge.capacity <= 0)
    return 999999.0;

  double ratio = static_cast<double>(edge.currentCars) / edge.capacity;

  // According to task.md: f(x) = 1 if x <= C, else f(x) = 1 + e^x
  // Let's assume C = 0.8 (starts getting congested when 80% full)
  double f_x = 1.0;
  if (ratio > 0.8) {
    f_x = 1.0 + std::exp(ratio - 0.8);
  }

  // time = c * L * f(n/v)
  double c = 0.1; // Base time constant
  return c * edge.length * f_x;
}
