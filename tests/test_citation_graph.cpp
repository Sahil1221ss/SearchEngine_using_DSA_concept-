
#include <iostream>
#include "../src/graph/citation_graph.h"

using namespace std;

int main()
{
    Graph graph;

    try
    {
        CitationGraphStats stats =
            loadCitationGraph("data/processed/cleaned_papers.csv", graph);

        cout << "=== Citation Graph Measurement ===\n";

        cout << "Papers: "
             << stats.totalPapers << endl;

        cout << "Papers with no references: "
             << stats.papersWithNoReferences << endl;

        cout << "Total reference links: "
             << stats.totalReferenceLinks << endl;

        cout << "Internal reference links: "
             << stats.internalReferenceLinks << endl;

        cout << "Papers cited at least once: "
             << stats.papersCitedAtLeastOnce << endl;

        cout << "Maximum internal citations: "
             << stats.maxInternalCitations << endl;

        cout << "Average internal references: "
             << stats.averageInternalReferences << endl;

        cout << "Graph vertices: "
             << graph.getVertexCount() << endl;

        cout << "Graph edges: "
             << graph.getEdgeCount() << endl;
    }
    catch (const exception &e)
    {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}
