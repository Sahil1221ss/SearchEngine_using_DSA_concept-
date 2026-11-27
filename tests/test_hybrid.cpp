// Tests the citation signals and the hybrid (BM25 + citations) ranker.
//
//   test_hybrid <index_dir> <golden_tsv> <cleaned_papers.csv>
//
// Uses the golden file only for its queries and the pure-BM25 order.

#include "hybrid_ranker.h"
#include "citation_graph.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

struct GoldenQuery
{
    std::vector<std::string> terms;
    std::vector<std::string> paperIds; // pure BM25 order, best first
};

static int failures = 0;

static void check(bool ok, const std::string &name)
{
    std::cout << (ok ? "PASS  " : "FAIL  ") << name << "\n";

    if (!ok)
        failures++;
}

static std::vector<std::string> splitTabs(const std::string &line)
{
    std::vector<std::string> parts;
    std::string piece;
    std::istringstream ss(line);

    while (std::getline(ss, piece, '\t'))
        parts.push_back(piece);

    return parts;
}

static std::vector<std::string> idsOf(const SearchIndex &index, const std::vector<RankedHit> &hits)
{
    std::vector<std::string> ids;

    for (const RankedHit &h : hits)
        ids.push_back(index.docs[h.docIdx].paperId);

    return ids;
}

// The order a signal alone should give: signal descending, ties -> larger paper_id first
static std::vector<std::string> expectedBySignal(const SearchIndex &index,
                                                 std::vector<RankedHit> pool,
                                                 const std::vector<double> &signal,
                                                 size_t k)
{
    std::sort(pool.begin(), pool.end(), [&](const RankedHit &a, const RankedHit &b)
              {
        if (signal[a.docIdx] != signal[b.docIdx])
            return signal[a.docIdx] > signal[b.docIdx];

        return index.docs[a.docIdx].paperId > index.docs[b.docIdx].paperId; });

    if (pool.size() > k)
        pool.resize(k);

    return idsOf(index, pool);
}

static bool allInUnitRange(const std::vector<double> &v)
{
    for (double x : v)
        if (x < 0.0 || x > 1.0)
            return false;

    return true;
}

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        std::cerr << "Usage: " << argv[0] << " <index_dir> <golden_tsv> <cleaned_papers.csv>\n";
        return 2;
    }

    SearchIndex index;
    std::string error;

    if (!loadSearchIndex(argv[1], index, error))
    {
        std::cerr << "Error: " << error << "\n";
        return 2;
    }

    Graph graph;
    CitationGraphStats stats;

    try
    {
        stats = loadCitationGraph(argv[3], graph);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error loading graph: " << e.what() << "\n";
        return 2;
    }

    // ---- read the golden queries ----
    std::ifstream golden(argv[2]);

    if (!golden.is_open())
    {
        std::cerr << "Could not open " << argv[2] << "\n";
        return 2;
    }

    std::vector<std::string> order;
    std::map<std::string, GoldenQuery> queries;
    std::string line;

    while (std::getline(golden, line))
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        if (line.empty())
            continue;

        std::vector<std::string> p = splitTabs(line);

        if (p.size() != 5)
        {
            std::cerr << "Bad golden line: " << line << "\n";
            return 2;
        }

        if (queries.find(p[0]) == queries.end())
        {
            order.push_back(p[0]);
            std::istringstream termStream(p[1]);
            std::string term;

            while (termStream >> term)
                queries[p[0]].terms.push_back(term);
        }

        queries[p[0]].paperIds.push_back(p[3]);
    }

    const CitationSignals signals = buildCitationSignals(index, &graph);
    const CitationSignals noGraph = buildCitationSignals(index, nullptr);
    const size_t n = index.docs.size();

    // ---- 1. citation-count signal ----
    {
        check(signals.citations.size() == n && allInUnitRange(signals.citations),
              "citation signal has one value per paper, all within 0..1");

        auto range = std::minmax_element(signals.citations.begin(), signals.citations.end());
        check(*range.first == 0.0 && *range.second == 1.0, "citation signal spans exactly 0 to 1");

        size_t hi = 0, lo = 0;

        for (size_t i = 0; i < n; i++)
        {
            if (index.docs[i].citations > index.docs[hi].citations)
                hi = i;
            if (index.docs[i].citations < index.docs[lo].citations)
                lo = i;
        }

        check(signals.citations[hi] == 1.0 && signals.citations[lo] == 0.0,
              "most-cited paper scores 1, least-cited scores 0");

        bool monotone = true;

        for (size_t i = 0; i < n && monotone; i++)
            for (size_t j = 0; j < n && monotone; j++)
                if (index.docs[i].citations > index.docs[j].citations &&
                    !(signals.citations[i] > signals.citations[j]))
                    monotone = false;

        check(monotone, "more citations always means a higher signal (all pairs)");
    }

    // ---- 2. graph signal, cross-checked against the loader's own statistics ----
    {
        check(noGraph.graph.size() == n &&
                  std::all_of(noGraph.graph.begin(), noGraph.graph.end(),
                              [](double x)
                              { return x == 0.0; }),
              "without a graph the graph signal is all zero");

        check(signals.graph.size() == n && allInUnitRange(signals.graph),
              "graph signal has one value per paper, all within 0..1");

        size_t positive = 0, top = 0;

        for (size_t i = 0; i < n; i++)
        {
            if (signals.graph[i] > 0.0)
                positive++;
            if (signals.graph[i] > signals.graph[top])
                top = i;
        }

        check(positive == stats.papersCitedAtLeastOnce,
              "papers with graph signal > 0 equals papersCitedAtLeastOnce (" +
                  std::to_string(positive) + " vs " + std::to_string(stats.papersCitedAtLeastOnce) + ")");

        // count incoming edges of the top paper by brute force
        size_t incoming = 0;

        for (size_t i = 0; i < n; i++)
            for (const std::string &cited : graph.getNeighbors(index.docs[i].paperId))
                if (cited == index.docs[top].paperId)
                    incoming++;

        check(signals.graph[top] == 1.0 && incoming == stats.maxInternalCitations,
              "top graph-signal paper has 1.0 and its incoming count equals maxInternalCitations (" +
                  std::to_string(incoming) + " vs " + std::to_string(stats.maxInternalCitations) + ")");
    }

    // ---- 3. zero weights give exactly the pure BM25 order ----
    {
        bool allSame = true;

        for (const std::string &q : order)
        {
            const GoldenQuery &g = queries[q];
            std::vector<RankedHit> hits = rankHybrid(index, signals, g.terms, 10, 0.0, 0.0);

            if (idsOf(index, hits) != g.paperIds)
            {
                allSame = false;
                std::cout << "      differs for query: " << q << "\n";
            }
        }

        check(allSame, "weights 0/0 reproduce the golden BM25 order for all " +
                           std::to_string(order.size()) + " queries");
    }

    // ---- 4. a signal with weight 1 orders the BM25 candidate pool by that signal ----
    {
        bool citesOk = true, graphOk = true;

        for (const std::string &q : order)
        {
            const GoldenQuery &g = queries[q];
            std::vector<RankedHit> pool = rankBM25(index, g.terms, 100);

            if (idsOf(index, rankHybrid(index, signals, g.terms, 10, 1.0, 0.0)) !=
                expectedBySignal(index, pool, signals.citations, 10))
                citesOk = false;

            if (idsOf(index, rankHybrid(index, signals, g.terms, 10, 0.0, 1.0)) !=
                expectedBySignal(index, pool, signals.graph, 10))
                graphOk = false;
        }

        check(citesOk, "wCites = 1 returns the 10 most-cited papers of the BM25 top 100");
        check(graphOk, "wGraph = 1 returns the 10 highest graph-signal papers of the BM25 top 100");
    }

    // ---- 5. results never come from outside the BM25 pool ----
    {
        bool inside = true;

        for (const std::string &q : order)
        {
            const GoldenQuery &g = queries[q];
            std::unordered_set<std::string> pool;

            for (const std::string &id : idsOf(index, rankBM25(index, g.terms, 100)))
                pool.insert(id);

            std::vector<RankedHit> hits = rankHybrid(index, signals, g.terms, 10, 0.5, 0.25);

            if (hits.size() != 10)
                inside = false;

            for (const std::string &id : idsOf(index, hits))
                if (pool.count(id) == 0)
                    inside = false;
        }

        check(inside, "blended results are 10 papers, all from the BM25 top 100");
    }

    // ---- 6. scores come back best first and inside 0..1 ----
    {
        bool sorted = true;

        for (const std::string &q : order)
        {
            std::vector<RankedHit> hits = rankHybrid(index, signals, queries[q].terms, 10, 0.2, 0.1);

            for (size_t i = 0; i < hits.size(); i++)
            {
                if (hits[i].score < 0.0 || hits[i].score > 1.0)
                    sorted = false;

                if (i > 0 && hits[i].score > hits[i - 1].score)
                    sorted = false;
            }
        }

        check(sorted, "blended scores are within 0..1 and never increase down the list");
    }

    // ---- 7. edge cases and bad input ----
    {
        check(rankHybrid(index, signals, {"zzzznotaterm"}, 10, 0.15, 0.0).empty(),
              "unknown term gives no results");
        check(rankHybrid(index, signals, queries[order[0]].terms, 0, 0.15, 0.0).empty(),
              "k = 0 gives no results");

        auto throws = [&](double wc, double wg)
        {
            try
            {
                rankHybrid(index, signals, queries[order[0]].terms, 10, wc, wg);
            }
            catch (const std::invalid_argument &)
            {
                return true;
            }

            return false;
        };

        check(throws(-0.1, 0.0), "negative citation weight throws invalid_argument");
        check(throws(0.0, -0.1), "negative graph weight throws invalid_argument");
        check(throws(0.7, 0.4), "weights adding up to more than 1 throw invalid_argument");

        CitationSignals wrongSize;
        bool mismatchThrows = false;

        try
        {
            rankHybrid(index, wrongSize, queries[order[0]].terms, 10, 0.1, 0.0);
        }
        catch (const std::invalid_argument &)
        {
            mismatchThrows = true;
        }

        check(mismatchThrows, "signals that do not match the index throw invalid_argument");
    }

    std::cout << "\n"
              << (failures == 0 ? "ALL HYBRID TESTS PASSED" : "SOME HYBRID TESTS FAILED") << "\n";
    return failures == 0 ? 0 : 1;
}