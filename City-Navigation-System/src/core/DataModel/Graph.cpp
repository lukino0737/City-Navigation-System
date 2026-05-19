#include "Graph.h"
#include <fstream>
#include <iomanip>
#include <thread>
#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QMetaObject>
#include "../modules/DataGenerator.h"

namespace {
struct ParsedMapData {
    std::vector<Node> nodes;
    std::vector<Edge> edges;
};

ParsedMapData parseMapJSON(const json& j) {
    ParsedMapData data;
    for (const auto& item : j["nodes"]) {
        std::string name;
        if (item.contains("name") && item["name"].is_string())
            name = item["name"].get<std::string>();
        data.nodes.push_back({item["id"], item["x"], item["y"], name});
    }
    for (const auto& item : j["edges"]) {
        double centrality = 0.0;
        if (item.contains("centrality") && item["centrality"].is_number())
            centrality = item["centrality"].get<double>();
        data.edges.push_back({item["id"], item["source"], item["target"],
                              item["length"], item["capacity"], 0, centrality});
    }
    return data;
}
} // anonymous namespace

std::vector<Node> getNodes(Graph& graph) {
    return graph.getAllNodes();
}
std::vector<Edge> getEdges(Graph& graph) {
    return graph.getAllEdges();
}
std::vector<Edge> getEdgesFromNode(Graph& graph, int nodeId) {
    return graph.getEdgesFrom(nodeId);
}
Node getNodeById(Graph& graph, int nodeId) {
    return graph.getNode(nodeId);
}
Edge getEdgeById(Graph& graph, int edgeId) {
    return graph.getEdgeById(edgeId);
}

bool Graph::load(std::string filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) return false;

    json j;
    try {
        file >> j;
    } catch (const std::exception& e) {
        qWarning() << "Graph::load parse error:" << e.what();
        return false;
    }

    ParsedMapData data;
    try {
        data = parseMapJSON(j);
    } catch (const std::exception& e) {
        qWarning() << "Graph::load data error:" << e.what();
        return false;
    }

    std::lock_guard<std::shared_mutex> dataLock(m_dataMutex);
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    m_nodes.clear();
    m_edges.clear();
    for (auto& n : data.nodes) m_nodes.push_back(std::move(n));
    for (auto& e : data.edges) m_edges.push_back(std::move(e));
    rebuildIndices();
    return true;
}

void Graph::regenerateGraph(int nodeCount) {
    std::thread([this, nodeCount]() {
        if (generateAndSaveMap(nodeCount, "map_data.json")) {
            QMetaObject::invokeMethod(this, [this]() {
                this->load("map_data.json");
                emit this->graphRegenerated();
            }, Qt::QueuedConnection);
        }
    }).detach();
}

void Graph::reloadSimulationMap() {
    QDir cwdDir = QDir::current();
    while (!cwdDir.exists("map_data.json") && cwdDir.cdUp()) { }
    QString path = cwdDir.exists("map_data.json")
        ? cwdDir.absoluteFilePath("map_data.json")
        : "map_data.json";
    loadMapInBackground(path, -1);
}

void Graph::loadMapInBackground(const QString& filePath, int mapIndex) {
    std::thread([this, filePath, mapIndex]() {
        try {
            QFile file(filePath);
            if (!file.open(QIODevice::ReadOnly)) {
                QMetaObject::invokeMethod(this, [this]() { emit graphRegenerated(); }, Qt::QueuedConnection);
                return;
            }
            QByteArray raw = file.readAll();
            file.close();
            json j = json::parse(raw.toStdString(), nullptr, false);
            if (j.is_discarded()) {
                QMetaObject::invokeMethod(this, [this]() { emit graphRegenerated(); }, Qt::QueuedConnection);
                return;
            }

            auto data = parseMapJSON(j);

            QMetaObject::invokeMethod(this, [this, mapIndex, d = std::move(data)]() mutable {
                std::lock_guard<std::shared_mutex> dataLock(m_dataMutex);
                std::lock_guard<std::recursive_mutex> lock(m_mutex);
                m_nodes = std::move(d.nodes);
                m_edges = std::move(d.edges);
                rebuildIndices();
                m_currentMapIndex = mapIndex;
                emit currentMapIndexChanged();
                emit graphRegenerated();
            }, Qt::QueuedConnection);
        } catch (const std::exception& e) {
            qWarning() << "Graph::loadMapInBackground error loading" << filePath << ":" << e.what();
            QMetaObject::invokeMethod(this, [this]() { emit graphRegenerated(); }, Qt::QueuedConnection);
        }
    }).detach();
}

void Graph::refreshAvailableMaps() {
    QStringList maps, files;
    QDir dir(QCoreApplication::applicationDirPath());
    while (!dir.exists("map_data") && dir.cdUp()) { }
    if (dir.exists("map_data")) {
        QDir mapDir(dir.absoluteFilePath("map_data"));
        const auto fileInfos = mapDir.entryInfoList({"*.json"}, QDir::Files, QDir::Name);
        for (const auto& info : fileInfos) {
            maps.append(info.completeBaseName());
            files.append(info.absoluteFilePath());
        }
    }
    {
        QDir cwdDir = QDir::current();
        while (!cwdDir.exists("map_data.json") && cwdDir.cdUp()) { }
        if (cwdDir.exists("map_data.json")) {
            maps.append("map_data");
            files.append(cwdDir.absoluteFilePath("map_data.json"));
        }
    }

    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        m_availableMaps = maps;
        m_availableMapFiles = files;
    }
    emit availableMapsChanged();
}

void Graph::switchToMap(int index) {
    QString filePath;
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        if (index < 0 || index >= m_availableMapFiles.size()) return;
        if (index == m_currentMapIndex) return;
        filePath = m_availableMapFiles[index];
    }
    loadMapInBackground(filePath, index);
}

bool Graph::save(std::string path) {
    json j;
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        j["nodes"] = m_nodes;
        j["edges"] = m_edges;
    }
    std::ofstream file(path);
    if (!file.is_open()) return false;
    file << std::setw(4) << j << std::endl; 
    return true;
}

Node Graph::getNode(int id) {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    auto it = m_nodeIdToIndex.find(id);
    if (it != m_nodeIdToIndex.end()) return m_nodes[it->second];
    return Node{ -1, 0, 0, "" };
}

std::vector<Edge>& Graph::getEdgesFrom(int nodeId) {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    return m_adjList[nodeId];
}

Edge& Graph::getEdgeById(int edgeId) {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    auto it = m_edgeIdToIndex.find(edgeId);
    if (it != m_edgeIdToIndex.end()) return m_edges[it->second];
    static Edge nullEdge = { -1, -1, -1, 0, 0 };
    return nullEdge;
}

void Graph::updateEdgeTraffic(int edgeId, int carCount) {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    auto it = m_edgeIdToIndex.find(edgeId);
    if (it != m_edgeIdToIndex.end()) {
        m_edges[it->second].currentCars = carCount;
        int source = m_edges[it->second].source;
        for (auto& adjEdge : m_adjList[source]) {
            if (adjEdge.id == edgeId) {
                adjEdge.currentCars = carCount;
                break;
            }
        }
    }
}

void Graph::batchUpdateTraffic(const std::vector<int>& edgeIds, const std::vector<int>& carCounts) {
    if (edgeIds.size() != carCounts.size()) return;
    std::lock_guard<std::shared_mutex> dataLock(m_dataMutex);
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    for (size_t i = 0; i < edgeIds.size(); ++i) {
        int eid = edgeIds[i];
        int count = carCounts[i];
        auto it = m_edgeIdToIndex.find(eid);
        if (it != m_edgeIdToIndex.end()) {
            size_t idx = it->second;
            m_edges[idx].currentCars = count;
            int source = m_edges[idx].source;
            for (auto& adjEdge : m_adjList[source]) {
                if (adjEdge.id == eid) {
                    adjEdge.currentCars = count;
                    break;
                }
            }
        }
    }
}

void Graph::updateEdgeCentrality(int edgeId, double centrality) {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    auto it = m_edgeIdToIndex.find(edgeId);
    if (it != m_edgeIdToIndex.end()) {
        m_edges[it->second].centrality = centrality;
        int source = m_edges[it->second].source;
        for (auto& adjEdge : m_adjList[source]) {
            if (adjEdge.id == edgeId) {
                adjEdge.centrality = centrality;
                break;
            }
        }
    }
}

const std::vector<Node>& Graph::getAllNodes() {
    // Note: Caller MUST hold Graph::mutex() while using this reference!
    return m_nodes;
}

const std::vector<Edge>& Graph::getAllEdges() {
    // Note: Caller MUST hold Graph::mutex() while using this reference!
    return m_edges;
}

bool Graph::addNode(int id, double x, double y, const std::string& name) {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    m_nodes.push_back(Node{ id, x, y, name });
    m_nodeIdToIndex[id] = m_nodes.size() - 1;
    return true;
}

bool Graph::addEdge(int id, int source, int target, double length, double capacity, double centrality) {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    Edge newedge{ id, source, target, length, capacity, 0, centrality };
    m_adjList[source].push_back(newedge);
    m_edgeIdToIndex[id] = m_edges.size();
    m_edges.push_back(newedge);
    return true;
}

void Graph::rebuildIndices() {
    // Caller must hold m_mutex and m_dataMutex
    m_adjList.clear();
    m_nodeIdToIndex.clear();
    m_edgeIdToIndex.clear();
    for (size_t i = 0; i < m_nodes.size(); ++i) m_nodeIdToIndex[m_nodes[i].Node_id] = i;
    for (size_t i = 0; i < m_edges.size(); ++i) {
        m_edgeIdToIndex[m_edges[i].id] = i;
        m_adjList[m_edges[i].source].push_back(m_edges[i]);
    }
}
