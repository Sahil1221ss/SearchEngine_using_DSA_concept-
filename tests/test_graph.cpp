
#include <iostream>
#include "../src/dsa/graph.h"
#include "../src/graph/bfs.h"
#include "../src/graph/dfs.h"

using namespace std;

int main()
{
    Graph graph;

    graph.addVertex("A");
    graph.addVertex("B");
    graph.addVertex("C");
    graph.addVertex("D");
    graph.addVertex("E");

    graph.addEdge("A", "B");
    graph.addEdge("A", "C");
    graph.addEdge("B", "D");
    graph.addEdge("C", "E");

    cout << "BFS Traversal: ";

    vector<string> bfsResult = BFS(graph, "A");

    for (const string &paper : bfsResult)
        cout << paper << " ";

    cout << "\nDFS Traversal: ";

    vector<string> dfsResult = DFS(graph, "A");

    for (const string &paper : dfsResult)
        cout << paper << " ";

    cout << endl;

    return 0;
}
