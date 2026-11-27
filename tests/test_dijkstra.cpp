
#include "../src/dsa/graph.h"
#include "../src/graph/citation_graph.h"
#include "../src/graph/dijkstra.h"

#include <iostream>
#include <iomanip>
#include <cmath>

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

    bool tested = false;

    for (const string &source : graph.getVertices())
    {
        const auto &edges =
            graph.getWeightedNeighbors(source);

        if (edges.empty())
            continue;

        const WeightedEdge &edge = edges.front();

        string destination = edge.destination;

        PathResult result =
            dijkstra(graph, source, destination);

        cout << "Source: " << source << "\n";
        cout << "Destination: " << destination << "\n";

        if (!result.found)
        {
            cout << "TEST FAILED: Direct edge was not reachable\n";
            return 1;
        }

        cout << fixed << setprecision(6);
        cout << "Path: ";

        for (const string &paper : result.path)
            cout << paper << " ";

        cout << "\nTotal cost: "
             << result.totalCost << "\n";

        if (result.path.size() < 2 ||
            result.path.front() != source ||
            result.path.back() != destination ||
            result.totalCost > edge.weight + 0.000001)
        {
            cout << "TEST FAILED\n";
            return 1;
        }

        cout << "\nREAL DATA DIJKSTRA TEST PASSED\n";

        tested = true;
        break;
    }

    if (!tested)
    {
        cout << "No citation edges found to test.\n";
        return 1;
    }

    return 0;
}
