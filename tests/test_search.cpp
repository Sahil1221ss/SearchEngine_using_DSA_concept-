// Checks the C++ BM25 ranker against the Python reference answers.
//
//   test_search <index_dir> <golden_tsv>
//
// golden_tsv lines:  query <TAB> stemmed terms <TAB> rank <TAB> paper_id <TAB> score
// (made by:  python tests/reference_bm25.py --tsv "query one" "query two" > tests/golden_bm25.tsv)

#include "result_ranker.h"

#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

struct Expected
{
    std::string paperId;
    double score;
};

struct GoldenQuery
{
    std::vector<std::string> terms;
    std::vector<Expected> hits;
};

static std::vector<std::string> splitTabs(const std::string &line)
{
    std::vector<std::string> parts;
    std::string piece;
    std::istringstream ss(line);

    while (std::getline(ss, piece, '\t'))
        parts.push_back(piece);

    return parts;
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <index_dir> <golden_tsv>\n";
        return 2;
    }

    SearchIndex index;
    std::string error;

    if (!loadSearchIndex(argv[1], index, error))
    {
        std::cerr << "Error: " << error << "\n";
        return 2;
    }

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

        queries[p[0]].hits.push_back({p[3], std::atof(p[4].c_str())});
    }

    std::cout << "Index: " << index.docs.size() << " documents, "
              << index.postings.size() << " terms\n\n";

    const double tolerance = 1e-4;
    int failed = 0;

    for (const std::string &query : order)
    {
        const GoldenQuery &g = queries[query];
        std::vector<RankedHit> got =
            rankBM25(index, g.terms, static_cast<int>(g.hits.size()));

        std::string problem;

        if (got.size() != g.hits.size())
        {
            problem = "expected " + std::to_string(g.hits.size()) +
                      " results, got " + std::to_string(got.size());
        }

        for (size_t i = 0; problem.empty() && i < got.size(); i++)
        {
            const DocInfo &doc = index.docs[got[i].docIdx];

            if (doc.paperId != g.hits[i].paperId)
            {
                problem = "rank " + std::to_string(i + 1) + ": expected " +
                          g.hits[i].paperId + ", got " + doc.paperId;
            }
            else if (std::fabs(got[i].score - g.hits[i].score) > tolerance)
            {
                problem = "rank " + std::to_string(i + 1) + ": score " +
                          std::to_string(got[i].score) + " vs expected " +
                          std::to_string(g.hits[i].score);
            }
        }

        if (problem.empty())
        {
            std::cout << "PASS  " << query << "\n";
        }
        else
        {
            std::cout << "FAIL  " << query << "  (" << problem << ")\n";
            failed++;
        }
    }

    std::cout << "\n"
              << (order.size() - failed) << " of " << order.size()
              << " queries match the reference\n";

    return failed == 0 ? 0 : 1;
}