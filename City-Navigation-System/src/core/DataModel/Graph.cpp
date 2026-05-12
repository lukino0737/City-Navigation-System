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
    file >> j;

    m_nodes.clear();
    m_edges.clear();
    m_adjList.clear();
    m_nodeIdToIndex.clear();
    m_edgeIdToIndex.clear();

    for (const auto& item : j["nodes"]) {
        std::string name;
        if (item.contains("name") && item["name"].is_string())
            name = item["name"].get<std::string>();
        addNode(item["id"], item["x"], item["y"], name);
    }
    for (const auto& item : j["edges"]) {
        addEdge(item["id"], item["source"], item["target"], item["length"], item["capacity"]);
    }
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
    // Search upward from CWD to find map_data.json
    QDir cwdDir = QDir::current();
    while (!cwdDir.exists("map_data.json") && cwdDir.cdUp()) { }
    QString path = cwdDir.exists("map_data.json")
        ? cwdDir.absoluteFilePath("map_data.json")
        : "map_data.json";

    std::thread([this, path]() {
        QMetaObject::invokeMethod(this, [this, path]() {
            this->load(path.toStdString());
            emit this->graphRegenerated();
        }, Qt::QueuedConnection);
    }).detach();
}

void Graph::refreshAvailableMaps() {
    m_availableMaps.clear();
    m_availableMapFiles.clear();

    // Search upward from exe directory until we find map_data/
    QDir dir(QCoreApplication::applicationDirPath());
    while (!dir.exists("map_data") && dir.cdUp()) { }
    if (dir.exists("map_data")) {
        QDir mapDir(dir.absoluteFilePath("map_data"));
        const auto files = mapDir.entryInfoList({"*.json"}, QDir::Files, QDir::Name);
        for (const auto& info : files) {
            m_availableMaps.append(info.completeBaseName());
            m_availableMapFiles.append(info.absoluteFilePath());
        }
    }

    // Also check map_data.json (generated map) — search upward from CWD
    {
        QDir cwdDir = QDir::current();
        while (!cwdDir.exists("map_data.json") && cwdDir.cdUp()) { }
        if (cwdDir.exists("map_data.json")) {
            m_availableMaps.append("map_data");
            m_availableMapFiles.append(cwdDir.absoluteFilePath("map_data.json"));
        }
    }

    emit availableMapsChanged();
}

void Graph::switchToMap(int index) {
    if (index < 0 || index >= m_availableMapFiles.size())
        return;
    if (index == m_currentMapIndex)
        return;

    QString filePath = m_availableMapFiles[index];

    std::thread([this, filePath, index]() {
        // Use QFile for Unicode path support (std::ifstream can't handle CJK paths on Windows)
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "switchToMap: failed to open" << filePath;
            QMetaObject::invokeMethod(this, [this]() {
                emit graphRegenerated();
            }, Qt::QueuedConnection);
            return;
        }

        QByteArray raw = file.readAll();
        file.close();

        json j = json::parse(raw.toStdString(), nullptr, false);
        if (j.is_discarded()) {
            qWarning() << "switchToMap: JSON parse failed for" << filePath;
            QMetaObject::invokeMethod(this, [this]() {
                emit graphRegenerated();
            }, Qt::QueuedConnection);
            return;
        }

        std::vector<Node> newNodes;
        std::vector<Edge> newEdges;
        for (const auto& item : j["nodes"]) {
            std::string name;
            if (item.contains("name") && item["name"].is_string())
                name = item["name"].get<std::string>();
            newNodes.push_back({item["id"], item["x"], item["y"], name});
        }
        for (const auto& item : j["edges"]) {
            newEdges.push_back({item["id"], item["source"], item["target"],
                               item["length"], item["capacity"], 0});
        }

        QMetaObject::invokeMethod(this, [this, index,
                n = std::move(newNodes), e = std::move(newEdges)]() mutable {
            m_nodes = std::move(n);
            m_edges = std::move(e);

            m_adjList.clear();
            m_nodeIdToIndex.clear();
            m_edgeIdToIndex.clear();

            for (size_t i = 0; i < m_nodes.size(); ++i)
                m_nodeIdToIndex[m_nodes[i].Node_id] = i;
            for (size_t i = 0; i < m_edges.size(); ++i) {
                m_edgeIdToIndex[m_edges[i].id] = i;
                m_adjList[m_edges[i].source].push_back(m_edges[i]);
            }

            // Only update currentIndex on success, after data is swapped
            m_currentMapIndex = index;
            emit currentMapIndexChanged();
            emit graphRegenerated();
        }, Qt::QueuedConnection);
    }).detach();
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
    return Node{ -1, 0, 0, "" };
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

bool Graph::addNode(int id, int x, int y, const std::string& name) {
    m_nodes.push_back(Node{ id, x, y, name });
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
