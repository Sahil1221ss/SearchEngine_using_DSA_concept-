
#include "dfs.h"
#include <unordered_set>

void DFSUtil(
    const Graph &graph,
    const string &current,
    unordered_set<string> &visited,
    vector<string> &result)
{
    visited.insert(current);
    result.push_back(current);

    for (const string &neighbor : graph.getNeighbors(current))
    {
        if (visited.find(neighbor) == visited.end())
        {
            DFSUtil(graph, neighbor, visited, result);
        }
    }
}

vector<string> DFS(const Graph &graph, const string &start)
{
    vector<string> result;

    if (!graph.hasVertex(start))
        return result;

    unordered_set<string> visited;

    DFSUtil(graph, start, visited, result);

    return result;
}
