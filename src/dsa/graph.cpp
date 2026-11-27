
#include "graph.h"
#include <stdexcept>

void Graph::addVertex(const string &vertex)
{
    adjacencyList.try_emplace(vertex);
    weightedAdjacencyList.try_emplace(vertex);
}

void Graph::addEdge(const string &source, const string &destination)
{
    addVertex(source);
    addVertex(destination);

    adjacencyList[source].push_back(destination);

    weightedAdjacencyList[source].push_back({destination, 1.0});

    edgeCount++;
}

void Graph::addWeightedEdge(
    const string &source,
    const string &destination,
    double weight)
{
    if (weight < 0.0)
        throw invalid_argument("Edge weight cannot be negative");

    addVertex(source);
    addVertex(destination);

    adjacencyList[source].push_back(destination);

    weightedAdjacencyList[source].push_back({destination, weight});

    edgeCount++;
}

bool Graph::hasVertex(const string &vertex) const
{
    return adjacencyList.find(vertex) != adjacencyList.end();
}

const vector<string> &Graph::getNeighbors(const string &vertex) const
{
    static const vector<string> empty;

    auto it = adjacencyList.find(vertex);

    if (it == adjacencyList.end())
        return empty;

    return it->second;
}

const vector<WeightedEdge> &Graph::getWeightedNeighbors(
    const string &vertex) const
{
    static const vector<WeightedEdge> empty;

    auto it = weightedAdjacencyList.find(vertex);

    if (it == weightedAdjacencyList.end())
        return empty;

    return it->second;
}

size_t Graph::getVertexCount() const
{
    return adjacencyList.size();
}

size_t Graph::getEdgeCount() const
{
    return edgeCount;
}

vector<string> Graph::getVertices() const
{
    vector<string> vertices;

    for (const auto &entry : adjacencyList)
        vertices.push_back(entry.first);

    return vertices;
}
