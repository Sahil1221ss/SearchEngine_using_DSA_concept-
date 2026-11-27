
#ifndef GRAPH_H
#define GRAPH_H

#include <unordered_map>
#include <vector>
#include <string>

using namespace std;

struct WeightedEdge
{
    string destination;
    double weight;
};

class Graph
{
private:
    unordered_map<string, vector<string>> adjacencyList;

    unordered_map<string, vector<WeightedEdge>> weightedAdjacencyList;

    size_t edgeCount = 0;

public:
    void addVertex(const string &vertex);

    void addEdge(const string &source, const string &destination);

    void addWeightedEdge(
        const string &source,
        const string &destination,
        double weight);

    vector<string> getVertices() const;

    bool hasVertex(const string &vertex) const;

    const vector<string> &getNeighbors(const string &vertex) const;

    const vector<WeightedEdge> &getWeightedNeighbors(
        const string &vertex) const;

    size_t getVertexCount() const;

    size_t getEdgeCount() const;
};

#endif
