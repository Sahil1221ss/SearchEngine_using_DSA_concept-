
#include "dijkstra.h"

#include <queue>
#include <unordered_map>
#include <limits>
#include <functional>
#include <algorithm>

PathResult dijkstra(
    const Graph &graph,
    const std::string &source,
    const std::string &destination)
{
    PathResult result;

    if (!graph.hasVertex(source) ||
        !graph.hasVertex(destination))
    {
        return result;
    }

    using QueueNode = std::pair<double, std::string>;

    std::priority_queue<
        QueueNode,
        std::vector<QueueNode>,
        std::greater<QueueNode>>
        pq;

    std::unordered_map<std::string, double> distance;
    std::unordered_map<std::string, std::string> previous;

    distance[source] = 0.0;
    pq.push({0.0, source});

    while (!pq.empty())
    {
        double currentDistance = pq.top().first;
        std::string current = pq.top().second;
        pq.pop();

        if (currentDistance > distance[current])
            continue;

        if (current == destination)
            break;

        for (const auto &edge : graph.getWeightedNeighbors(current))
        {
            double newDistance =
                currentDistance + edge.weight;

            if (distance.find(edge.destination) == distance.end())
                distance[edge.destination] =
                    std::numeric_limits<double>::infinity();

            if (newDistance < distance[edge.destination])
            {
                distance[edge.destination] = newDistance;
                previous[edge.destination] = current;

                pq.push({newDistance, edge.destination});
            }
        }
    }

    if (distance.find(destination) == distance.end() ||
        distance[destination] ==
            std::numeric_limits<double>::infinity())
    {
        return result;
    }

    result.found = true;
    result.totalCost = distance[destination];

    std::string current = destination;

    while (current != source)
    {
        result.path.push_back(current);
        current = previous[current];
    }

    result.path.push_back(source);

    std::reverse(result.path.begin(), result.path.end());

    return result;
}
