
#include "graph/citation_graph.h"
#include "graph/dijkstra.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <stdexcept>

using namespace std;

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        cout << "Usage: citation_path_cli <csv_path> <source_id> <destination_id>\n";
        return 1;
    }

    string csvPath = argv[1];
    string source = argv[2];
    string destination = argv[3];

    try
    {
        Graph graph;

        CitationGraphStats stats =
            loadCitationGraph(csvPath, graph);

        cout << "Citation graph loaded successfully.\n";
        cout << "Papers: " << stats.totalPapers << "\n";
        cout << "Internal citation edges: "
             << stats.internalReferenceLinks << "\n\n";

        if (!graph.hasVertex(source))
        {
            cout << "Source paper not found in dataset.\n";
            return 1;
        }

        if (!graph.hasVertex(destination))
        {
            cout << "Destination paper not found in dataset.\n";
            return 1;
        }

        PathResult result =
            dijkstra(graph, source, destination);

        if (!result.found)
        {
            cout << "No citation path exists between these papers.\n";
            return 0;
        }

        cout << fixed << setprecision(6);

        cout << "Shortest citation path:\n";

        for (size_t i = 0; i < result.path.size(); i++)
        {
            cout << result.path[i];

            if (i + 1 < result.path.size())
                cout << "\n    -> ";
        }

        cout << "\n\nTotal path cost: "
             << result.totalCost << "\n";

        cout << "Number of edges: "
             << result.path.size() - 1 << "\n";
    }
    catch (const exception &e)
    {
        cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
