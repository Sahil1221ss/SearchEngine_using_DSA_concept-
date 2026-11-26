#include "graph.h"
#include <algorithm>

void Graph::addVertex(const string &paperID)
{
    adjacencyList.try_emplace(paperID);
}

void Graph::addEdge(const string &from, const string &to)
{
    if (!hasVertex(from) || !hasVertex(to))
        return;

    vector<string> &neighbors = adjacencyList[from];

    if (find(neighbors.begin(), neighbors.end(), to) == neighbors.end())
    {
        neighbors.push_back(to);
    }
}

bool Graph::hasVertex(const string &paperID) const
{
    return adjacencyList.find(paperID) != adjacencyList.end();
}

const vector<string> &Graph::getNeighbors(const string &paperID) const
{
    static const vector<string> empty;

    auto it = adjacencyList.find(paperID);

    if (it == adjacencyList.end())
        return empty;

    return it->second;
}

int Graph::getVertexCount() const
{
    return static_cast<int>(adjacencyList.size());
}

int Graph::getEdgeCount() const
{
    int count = 0;

    for (const auto &entry : adjacencyList)
    {
        count += static_cast<int>(entry.second.size());
    }

    return count;
}