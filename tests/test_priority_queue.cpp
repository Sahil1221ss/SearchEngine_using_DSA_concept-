
#include "../src/dsa/priority_queue.h"
#include <iostream>
using namespace std;

int main()
{
    MinHeap pq(3);

    pq.offer({"PAPER_1", 4.5});
    pq.offer({"PAPER_2", 9.2});
    pq.offer({"PAPER_3", 2.1});
    pq.offer({"PAPER_4", 7.8});
    pq.offer({"PAPER_5", 2.1});

    cout << "Top 3 papers:\n";

    while (!pq.empty())
    {
        HeapNode paper = pq.extractMin();

        cout << paper.paper_id
             << " : " << paper.score << endl;
    }

    return 0;
}
