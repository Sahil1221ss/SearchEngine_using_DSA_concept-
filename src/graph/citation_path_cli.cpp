
#include "citation_graph.h"
#include "dijkstra.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <exception>

using namespace std;

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        cerr << "Usage: citation_path_cli <csv_path> <source_id> <destination_id>\n";
        return 1;
    }

    string csvPath = argv[1];
    string source = argv[2];
    string destination = argv[3];

    try
    {
        Graph graph;

        loadCitationGraph(csvPath, graph);

        PathResult result = dijkstra(
            graph,
            source,
            destination);

        if (!result.found)
        {
            cout << "NOT_FOUND\n";
            return 0;
        }

        cout << "FOUND\t"
             << fixed << setprecision(6)
             << result.totalCost << "\n";

        cout << "PATH";

        for (const string &paperID : result.path)
        {
            cout << "\t" << paperID;
        }

        cout << "\n";
    }
    catch (const exception &e)
    {
        cerr << "Citation path error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
