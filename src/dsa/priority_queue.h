
#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include <vector>
#include <string>
using namespace std;

struct HeapNode
{
    string paper_id;
    double score;
};

class MinHeap
{
private:
    vector<HeapNode> heap;
    int capacity;

    int parent(int i);
    int leftChild(int i);
    int rightChild(int i);

    bool isSmaller(const HeapNode &a, const HeapNode &b);

    void siftUp(int i);
    void siftDown(int i);

public:
    MinHeap(int k);

    void insert(const HeapNode &node);
    HeapNode getMin();
    HeapNode extractMin();

    void offer(const HeapNode &node);

    int size();
    bool empty();
    bool isFull();
};

#endif
