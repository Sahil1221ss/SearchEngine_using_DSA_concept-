
#include "../src/dsa/graph.h"
#include "../src/graph/citation_graph.h"

#include <iostream>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <algorithm>

using namespace std;

int main()
{
    Graph graph;

    CitationGraphStats stats =
        loadCitationGraph(
            "data/processed/cleaned_papers.csv",
            graph);

    cout << "Papers: " << stats.totalPapers << "\n";
    cout << "Internal edges: "
         << stats.internalReferenceLinks << "\n\n";

    // Create a temporary undirected adjacency list
    unordered_map<string, vector<string>> undirected;

    for (const string &vertex : graph.getVertices())
    {
        undirected[vertex];

        for (const string &neighbor : graph.getNeighbors(vertex))
        {
            undirected[vertex].push_back(neighbor);
            undirected[neighbor].push_back(vertex);
        }
    }

    unordered_set<string> visited;
    vector<size_t> componentSizes;

    // Find weakly connected components using BFS
    for (const string &start : graph.getVertices())
    {
        if (visited.count(start))
            continue;

        queue<string> q;
        q.push(start);
        visited.insert(start);

        size_t componentSize = 0;

        while (!q.empty())
        {
            string current = q.front();
            q.pop();

            componentSize++;

            for (const string &neighbor : undirected[current])
            {
                if (!visited.count(neighbor))
                {
                    visited.insert(neighbor);
                    q.push(neighbor);
                }
            }
        }

        componentSizes.push_back(componentSize);
    }

    sort(
        componentSizes.begin(),
        componentSizes.end(),
        greater<size_t>());

    size_t isolatedPapers = 0;

    for (size_t size : componentSizes)
    {
        if (size == 1)
            isolatedPapers++;
    }

    cout << "Weakly connected components: "
         << componentSizes.size() << "\n";

    cout << "Isolated papers: "
         << isolatedPapers << "\n";

    cout << "Largest component: "
         << componentSizes.front() << " papers\n";

    cout << "Component sizes (largest first):\n";

    for (size_t size : componentSizes)
        cout << size << " ";

    cout << "\n";

    return 0;
}
