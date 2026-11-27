
#include "priority_queue.h"
#include <stdexcept>
#include <utility>

MinHeap::MinHeap(int k)
{
    if (k <= 0)
        throw invalid_argument("Heap capacity must be positive");

    capacity = k;
}

int MinHeap::parent(int i)
{
    return (i - 1) / 2;
}

int MinHeap::leftChild(int i)
{
    return 2 * i + 1;
}

int MinHeap::rightChild(int i)
{
    return 2 * i + 2;
}

// Returns true if a should appear closer to the root
bool MinHeap::isSmaller(const HeapNode &a, const HeapNode &b)
{
    if (a.score != b.score)
        return a.score < b.score;

    return a.paper_id < b.paper_id;
}

void MinHeap::siftUp(int i)
{
    while (i > 0)
    {
        int p = parent(i);

        if (isSmaller(heap[i], heap[p]))
        {
            swap(heap[i], heap[p]);
            i = p;
        }
        else
        {
            break;
        }
    }
}

void MinHeap::siftDown(int i)
{
    int n = heap.size();

    while (true)
    {
        int smallest = i;
        int left = leftChild(i);
        int right = rightChild(i);

        if (left < n && isSmaller(heap[left], heap[smallest]))
            smallest = left;

        if (right < n && isSmaller(heap[right], heap[smallest]))
            smallest = right;

        if (smallest == i)
            break;

        swap(heap[i], heap[smallest]);
        i = smallest;
    }
}

void MinHeap::insert(const HeapNode &node)
{
    if (isFull())
        throw overflow_error("Heap is full");

    heap.push_back(node);
    siftUp(heap.size() - 1);
}

HeapNode MinHeap::getMin()
{
    if (empty())
        throw underflow_error("Heap is empty");

    return heap[0];
}

HeapNode MinHeap::extractMin()
{
    if (empty())
        throw underflow_error("Heap is empty");

    HeapNode minimum = heap[0];

    heap[0] = heap.back();
    heap.pop_back();

    if (!empty())
        siftDown(0);

    return minimum;
}

void MinHeap::offer(const HeapNode &node)
{
    if (!isFull())
    {
        insert(node);
    }
    else if (isSmaller(heap[0], node))
    {
        heap[0] = node;
        siftDown(0);
    }
}

int MinHeap::size() const
{
    return static_cast<int>(heap.size());
}

bool MinHeap::empty() const
{
    return heap.empty();
}

bool MinHeap::isFull() const
{
    return heap.size() >= capacity;
}
