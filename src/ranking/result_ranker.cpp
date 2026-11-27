#include "result_ranker.h"
#include "priority_queue.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <sstream>

namespace
{
    // Splits on '\t' into at most maxParts pieces; the last piece keeps the rest.
    std::vector<std::string> splitTabs(const std::string &line, size_t maxParts)
    {
        std::vector<std::string> parts;
        size_t start = 0;

        while (parts.size() + 1 < maxParts)
        {
            size_t tab = line.find('\t', start);

            if (tab == std::string::npos)
                break;

            parts.push_back(line.substr(start, tab - start));
            start = tab + 1;
        }

        parts.push_back(line.substr(start));
        return parts;
    }

    void stripCR(std::string &s)
    {
        if (!s.empty() && s.back() == '\r')
            s.pop_back();
    }

    // The ONLY place that builds a HeapNode. If your HeapNode in priority_queue.h
    // has different field names or types, edit this function and nothing else.
    HeapNode makeHeapNode(double score, const std::string &paperId)
    {
        HeapNode node;
        node.score = score;
        node.paper_id = paperId;
        return node;
    }
}

bool loadSearchIndex(const std::string &indexDir,
                     SearchIndex &index,
                     std::string &error)
{
    index = SearchIndex();

    // ---- docs.tsv: doc_idx  paper_id  weighted_len  citations  year  title ----
    const std::string docsPath = indexDir + "/docs.tsv";
    std::ifstream docsFile(docsPath);

    if (!docsFile.is_open())
    {
        error = "Could not open " + docsPath;
        return false;
    }

    std::string line;
    long long totalLength = 0;

    while (std::getline(docsFile, line))
    {
        stripCR(line);

        if (line.empty())
            continue;

        std::vector<std::string> parts = splitTabs(line, 6);

        if (parts.size() != 6 ||
            std::atoi(parts[0].c_str()) != static_cast<int>(index.docs.size()))
        {
            error = "Bad line in docs.tsv: " + line.substr(0, 60);
            return false;
        }

        DocInfo doc;
        doc.paperId = parts[1];
        doc.length = std::atoi(parts[2].c_str());
        doc.citations = std::atoi(parts[3].c_str());
        doc.year = parts[4];
        doc.title = parts[5];

        index.paperToIdx[doc.paperId] = static_cast<int>(index.docs.size());
        totalLength += doc.length;
        index.docs.push_back(std::move(doc));
    }

    if (index.docs.empty())
    {
        error = "docs.tsv has no documents";
        return false;
    }

    index.avgLength = static_cast<double>(totalLength) / index.docs.size();

    // ---- postings.tsv: term  df  doc_idx:tf doc_idx:tf ... ----
    const std::string postingsPath = indexDir + "/postings.tsv";
    std::ifstream postingsFile(postingsPath);

    if (!postingsFile.is_open())
    {
        error = "Could not open " + postingsPath;
        return false;
    }

    const int docCount = static_cast<int>(index.docs.size());

    while (std::getline(postingsFile, line))
    {
        stripCR(line);

        if (line.empty())
            continue;

        std::vector<std::string> parts = splitTabs(line, 3);

        if (parts.size() != 3)
        {
            error = "Bad line in postings.tsv: " + line.substr(0, 60);
            return false;
        }

        std::vector<std::pair<int, int>> list;
        std::istringstream cells(parts[2]);
        std::string cell;

        while (cells >> cell)
        {
            size_t colon = cell.find(':');

            if (colon == std::string::npos)
            {
                error = "Bad posting '" + cell + "' for term " + parts[0];
                return false;
            }

            int doc = std::atoi(cell.substr(0, colon).c_str());
            int tf = std::atoi(cell.substr(colon + 1).c_str());

            if (doc < 0 || doc >= docCount || tf <= 0)
            {
                error = "Out-of-range posting '" + cell + "' for term " + parts[0];
                return false;
            }

            list.push_back({doc, tf});
        }

        index.postings[parts[0]] = std::move(list);
    }

    return true;
}

std::vector<RankedHit> rankBM25(const SearchIndex &index,
                                const std::vector<std::string> &stemmedTerms,
                                int k,
                                double k1,
                                double b)
{
    std::vector<RankedHit> result;

    if (k <= 0 || index.docs.empty())
        return result;

    // Distinct terms in sorted order, so sums are added in the same order as
    // tests/reference_bm25.py and every run gives identical numbers.
    std::vector<std::string> terms(stemmedTerms);
    std::sort(terms.begin(), terms.end());
    terms.erase(std::unique(terms.begin(), terms.end()), terms.end());

    const double n = static_cast<double>(index.docs.size());

    std::vector<double> score(index.docs.size(), 0.0);
    std::vector<int> touched; // docs that matched at least one term

    for (const std::string &term : terms)
    {
        auto it = index.postings.find(term);

        if (it == index.postings.end())
            continue;

        const double df = static_cast<double>(it->second.size());
        const double idf = std::log(1.0 + (n - df + 0.5) / (df + 0.5));

        for (const auto &posting : it->second)
        {
            const int doc = posting.first;
            const double tf = static_cast<double>(posting.second);
            const double norm =
                tf + k1 * (1.0 - b + b * index.docs[doc].length / index.avgLength);

            // Every contribution is > 0, so score[doc] == 0.0 means "first time seen"
            if (score[doc] == 0.0)
                touched.push_back(doc);

            score[doc] += idf * tf * (k1 + 1.0) / norm;
        }
    }

    // Keep only the best k in a min-heap: the root is always the worst of the kept hits.
    MinHeap heap(k);

    for (int doc : touched)
        heap.offer(makeHeapNode(score[doc], index.docs[doc].paperId));

    // extractMin() returns worst first, so reverse to get best first
    while (!heap.empty())
    {
        HeapNode node = heap.extractMin();
        result.push_back({index.paperToIdx.at(node.paper_id), node.score});
    }

    std::reverse(result.begin(), result.end());
    return result;
}