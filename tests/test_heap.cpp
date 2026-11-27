// Tests the custom MinHeap against a plain sort.
//
// The reference rule must match MinHeap::isSmaller(): higher score is better,
// and for equal scores the LARGER paper_id is better.
//
// If your HeapNode differs from {double score; string paper_id;}, edit makeNode().

#include "priority_queue.h"

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

static HeapNode makeNode(double score, const std::string &id)
{
    HeapNode node;
    node.score = score;
    node.paper_id = id;
    return node;
}

static std::string makeId(int n)
{
    char buf[32];
    std::snprintf(buf, sizeof(buf), "id%06d", n);
    return buf;
}

static bool better(const HeapNode &a, const HeapNode &b)
{
    if (a.score != b.score)
        return a.score > b.score;

    return a.paper_id > b.paper_id;
}

static int failures = 0;

static void check(bool ok, const std::string &name)
{
    std::cout << (ok ? "PASS  " : "FAIL  ") << name << "\n";

    if (!ok)
        failures++;
}

template <typename ExceptionType, typename Fn>
static bool throwsType(Fn fn)
{
    try
    {
        fn();
    }
    catch (const ExceptionType &)
    {
        return true;
    }
    catch (...)
    {
        return false;
    }

    return false;
}

int main()
{
    // 1. Top-K through offer() must equal "sort everything, take the first K"
    std::mt19937 rng(12345);

    for (int k : {1, 2, 5, 10, 50, 300})
    {
        std::vector<HeapNode> all;

        // scores drawn from only 100 values, so there are many exact ties
        for (int i = 0; i < 5000; i++)
            all.push_back(makeNode((rng() % 100) / 10.0, makeId(i)));

        MinHeap heap(k);

        for (const HeapNode &node : all)
            heap.offer(node);

        std::vector<HeapNode> got;

        while (!heap.empty())
            got.push_back(heap.extractMin());

        std::reverse(got.begin(), got.end()); // worst-first -> best-first

        std::vector<HeapNode> want = all;
        std::sort(want.begin(), want.end(), better);
        want.resize(k);

        bool same = got.size() == want.size();

        for (size_t i = 0; same && i < got.size(); i++)
            same = got[i].score == want[i].score && got[i].paper_id == want[i].paper_id;

        check(same, "top-" + std::to_string(k) + " of 5000 random nodes with many ties");
    }

    // 2. Fewer items than capacity
    {
        MinHeap heap(10);
        heap.offer(makeNode(3.0, "a"));
        heap.offer(makeNode(1.0, "b"));
        heap.offer(makeNode(2.0, "c"));

        check(heap.size() == 3 && !heap.isFull(), "size is 3 when capacity is 10");
        check(heap.getMin().paper_id == "b", "getMin returns the lowest score");
        check(heap.extractMin().paper_id == "b" && heap.extractMin().paper_id == "c" &&
                  heap.extractMin().paper_id == "a" && heap.empty(),
              "extractMin returns ascending order and empties the heap");
    }

    // 3. A worse item must not replace anything once the heap is full
    {
        MinHeap heap(2);
        heap.offer(makeNode(5.0, "x"));
        heap.offer(makeNode(4.0, "y"));
        heap.offer(makeNode(1.0, "z"));

        check(heap.size() == 2 && heap.getMin().paper_id == "y",
              "a lower score is rejected when full");
    }

    // 4. Error handling
    check(throwsType<std::invalid_argument>([]
                                            { MinHeap h(0); }),
          "capacity 0 throws invalid_argument");
    check(throwsType<std::underflow_error>([]
                                           { MinHeap h(3); h.extractMin(); }),
          "extractMin on empty throws underflow_error");
    check(throwsType<std::underflow_error>([]
                                           { MinHeap h(3); h.getMin(); }),
          "getMin on empty throws underflow_error");
    check(throwsType<std::overflow_error>([]
                                          {
              MinHeap h(1);
              h.insert(makeNode(1.0, "a"));
              h.insert(makeNode(2.0, "b")); }),
          "insert on a full heap throws overflow_error");

    std::cout << "\n"
              << (failures == 0 ? "ALL HEAP TESTS PASSED" : "SOME HEAP TESTS FAILED") << "\n";
    return failures == 0 ? 0 : 1;
}