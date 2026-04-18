#pragma once

#include <vector>
#include "../../api/NavigationAPI.h"
#include<queue>
#include "../DataModel/Graph.h"

class SpatialSearch {
private:
    Graph* m_graph;
public:
    SpatialSearch(Graph* graph);
    ~SpatialSearch();
    std::vector<Node> getNearestPoints(double x, double y, int count);
};
