
#include "bfs.h"
#include <queue>
#include <unordered_set>

vector<string> BFS(const Graph &graph, const string &start)
{
    vector<string> result;

    if (!graph.hasVertex(start))
        return result;

    queue<string> q;
    unordered_set<string> visited;

    q.push(start);
    visited.insert(start);

    while (!q.empty())
    {
        string current = q.front();
        q.pop();

        result.push_back(current);

        for (const string &neighbor : graph.getNeighbors(current))
        {
            if (visited.find(neighbor) == visited.end())
            {
                visited.insert(neighbor);
                q.push(neighbor);
            }
        }
    }

    return result;
}
