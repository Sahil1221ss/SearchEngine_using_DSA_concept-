#ifndef RESULT_RANKER_H
#define RESULT_RANKER_H

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

// One paper, as written to data/index/docs.tsv by export_for_cpp.py
struct DocInfo
{
    std::string paperId;
    int length = 0;    // weighted token count (title x5, topics x2, abstract x1)
    int citations = 0; // OpenAlex cited-by count
    std::string year;
    std::string title;
};

// The whole search index, loaded from data/index/docs.tsv and postings.tsv
struct SearchIndex
{
    std::vector<DocInfo> docs;

    // term -> list of (docIdx, weighted term frequency), docIdx ascending
    std::unordered_map<std::string, std::vector<std::pair<int, int>>> postings;

    // paperId -> docIdx
    std::unordered_map<std::string, int> paperToIdx;

    double avgLength = 0.0;
};

struct RankedHit
{
    int docIdx;
    double score;
};

// Loads the index files from indexDir. Returns false and fills `error` on failure.
bool loadSearchIndex(const std::string &indexDir,
                     SearchIndex &index,
                     std::string &error);

// BM25 over already-stemmed query terms (OR semantics: any matching term scores).
// Returns at most k hits, best first. Uses MinHeap, so the cost is O(n log k).
std::vector<RankedHit> rankBM25(const SearchIndex &index,
                                const std::vector<std::string> &stemmedTerms,
                                int k,
                                double k1 = 1.2,
                                double b = 0.75);

#endif