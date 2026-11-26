#include <iostream>
#include "../src/dsa/graph.h"

using namespace std;

int main()
{
    Graph graph;

    string A = "Paper_A";
    string B = "Paper_B";
    string C = "Paper_C";

    graph.addVertex(A);
    graph.addVertex(B);
    graph.addVertex(C);

    graph.addEdge(A, B);
    graph.addEdge(A, C);
    graph.addEdge(B, C);

    cout << "Vertices: " << graph.getVertexCount() << endl;
    cout << "Edges: " << graph.getEdgeCount() << endl;

    cout << "\nPapers referenced by A:\n";

    for (const string &paper : graph.getNeighbors(A))
    {
        cout << paper << endl;
    }

    return 0;
}