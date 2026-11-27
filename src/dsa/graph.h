#ifndef GRAPH_H
#define GRAPH_H

#include <string>
#include <vector>
#include <unordered_map>

using namespace std;

class Graph
{
private:
    unordered_map<string, vector<string>> adjacencyList;

public:
    void addVertex(const string &paperID);

    void addEdge(const string &from, const string &to);

    bool hasVertex(const string &paperID) const;

    const vector<string> &getNeighbors(const string &paperID) const;

    int getVertexCount() const;

    int getEdgeCount() const;
};

#endif