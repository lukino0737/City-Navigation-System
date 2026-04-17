#pragma once
#include <string>

class Graph;

class DataGenerator {
public:
    DataGenerator();
    ~DataGenerator();
};

// 全局接口声明
bool generateAndSaveMap(int nodeCount, std::string filePath);
bool loadMap(Graph& graph, std::string filePath);
