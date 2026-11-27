
#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "../dsa/graph.h"
#include <string>
#include <vector>

struct PathResult
{
    bool found = false;
    double totalCost = 0.0;
    std::vector<std::string> path;
};

PathResult dijkstra(
    const Graph &graph,
    const std::string &source,
    const std::string &destination);

#endif
