#include "hybrid_ranker.h"
#include "graph.h"
#include "priority_queue.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace
{
    // Builds a HeapNode by field name. If your HeapNode ever changes, edit only this.
    HeapNode makeHeapNode(double score, const std::string &paperId)
    {
        HeapNode node;
        node.score = score;
        node.paper_id = paperId;
        return node;
    }

    // Scales so the smallest value becomes 0 and the largest becomes 1.
    // If every value is equal there is no information, so everything becomes 0.
    void scaleToUnit(std::vector<double> &values)
    {
        if (values.empty())
            return;

        auto range = std::minmax_element(values.begin(), values.end());
        const double low = *range.first;
        const double high = *range.second;

        for (double &v : values)
            v = (high > low) ? (v - low) / (high - low) : 0.0;
    }
}

CitationSignals buildCitationSignals(const SearchIndex &index, const Graph *graph)
{
    const size_t n = index.docs.size();
    CitationSignals signals;

    // Signal 1: OpenAlex citation count. log() tames the huge spread
    // (median ~430 citations, maximum >100,000) before scaling.
    signals.citations.resize(n);

    for (size_t i = 0; i < n; i++)
        signals.citations[i] = std::log1p(std::max(0, index.docs[i].citations));

    scaleToUnit(signals.citations);

    // Signal 2: how many papers inside this corpus cite the paper, from the graph
    signals.graph.assign(n, 0.0);

    if (graph != nullptr)
    {
        std::vector<int> incoming(n, 0);

        for (size_t i = 0; i < n; i++)
        {
            // an edge A -> B means "A cites B", so B gains one incoming citation
            for (const std::string &cited : graph->getNeighbors(index.docs[i].paperId))
            {
                auto it = index.paperToIdx.find(cited);

                if (it != index.paperToIdx.end())
                    incoming[it->second]++;
            }
        }

        for (size_t i = 0; i < n; i++)
            signals.graph[i] = std::log1p(incoming[i]);

        scaleToUnit(signals.graph);
    }

    return signals;
}

std::vector<RankedHit> rankHybrid(const SearchIndex &index,
                                  const CitationSignals &signals,
                                  const std::vector<std::string> &stemmedTerms,
                                  int k,
                                  double wCites,
                                  double wGraph,
                                  int candidates)
{
    if (wCites < 0.0 || wGraph < 0.0 || wCites + wGraph > 1.0)
        throw std::invalid_argument("weights must be >= 0 and wCites + wGraph <= 1");

    if (signals.citations.size() != index.docs.size() ||
        signals.graph.size() != index.docs.size())
        throw std::invalid_argument("citation signals do not match the index");

    std::vector<RankedHit> result;

    if (k <= 0)
        return result;

    // Step 1: BM25 picks the candidate pool (best first)
    std::vector<RankedHit> pool = rankBM25(index, stemmedTerms, std::max(candidates, k));

    if (pool.empty())
        return result;

    const double bestBm25 = pool.front().score;
    const double wBm25 = 1.0 - wCites - wGraph;

    // Step 2: blend and keep the best k in the min-heap
    MinHeap heap(k);

    for (const RankedHit &hit : pool)
    {
        const double bm25 = (bestBm25 > 0.0) ? hit.score / bestBm25 : 0.0;

        const double blended = wBm25 * bm25 +
                               wCites * signals.citations[hit.docIdx] +
                               wGraph * signals.graph[hit.docIdx];

        heap.offer(makeHeapNode(blended, index.docs[hit.docIdx].paperId));
    }

    // extractMin() returns worst first, so reverse to get best first
    while (!heap.empty())
    {
        HeapNode node = heap.extractMin();
        result.push_back({index.paperToIdx.at(node.paper_id), node.score});
    }

    std::reverse(result.begin(), result.end());
    return result;
}