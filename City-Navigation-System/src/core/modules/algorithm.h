#pragma once
#include <vector>
#include "../../api/NavigationAPI.h" 

PathResult calculateShortestPath(int startId, int endId, bool considerTraffic, Graph& graph);
