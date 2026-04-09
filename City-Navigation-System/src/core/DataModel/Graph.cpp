#include "Graph.h"
#include <fstream>
#include <iomanip>


Graph::Graph() {}
Graph::~Graph() {}

bool Graph::load(std::string filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) return false;

    json j;
    file >> j;

    m_nodes.clear();
    m_edges.clear();
    m_adjList.clear();
    m_nodeIdToIndex.clear();
    m_edgeIdToIndex.clear();

    for (const auto& item : j["nodes"]) {
        addNode(item["id"], item["x"], item["y"]);
    }
    for (const auto& item : j["edges"]) {
        addEdge(item["id"], item["source"], item["target"], item["length"], item["capacity"]);
    }
    return true;
}

bool Graph::save(std::string path) {
    json j;
    j["nodes"] = m_nodes;
    j["edges"] = m_edges;

    std::ofstream file(path);
    if (!file.is_open()) return false;

    file << std::setw(4) << j << std::endl; 
    return true;
}

Node Graph::getNode(int id) {
    auto it = m_nodeIdToIndex.find(id);
    if (it != m_nodeIdToIndex.end()) {
        return m_nodes[it->second];
    }
    return Node{ -1, 0, 0 };
}

std::vector<Edge>& Graph::getEdgesFrom(int nodeId) {
    return m_adjList[nodeId];
}

Edge& Graph::getEdgeById(int edgeId) {
    auto it = m_edgeIdToIndex.find(edgeId);
    if (it != m_edgeIdToIndex.end()) {
        return m_edges[it->second];
    }
    return m_edges[0];
}

void Graph::updateEdgeTraffic(int edgeId, int carCount) {
    auto it = m_edgeIdToIndex.find(edgeId);
    if (it != m_edgeIdToIndex.end()) {
        m_edges[it->second].currentCars = carCount;
    }
}

const std::vector<Node>& Graph::getAllNodes() {
    return m_nodes;
}

const std::vector<Edge>& Graph::getAllEdges() {
    return m_edges;
}

bool Graph::addNode(int id, int x, int y) {
    m_nodes.push_back(Node{ id, x, y });
    m_nodeIdToIndex[id] = m_nodes.size() - 1;
    return true;
}

bool Graph::addEdge(int id, int source, int target, double length, double capacity) {
    Edge newedge{ id, source, target, length, capacity, 0 };
    m_adjList[source].push_back(newedge);
    m_edgeIdToIndex[id] = m_edges.size();
    m_edges.push_back(newedge);
    return true;
}
