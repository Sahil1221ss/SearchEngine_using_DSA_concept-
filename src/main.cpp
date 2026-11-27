// Command-line search: BM25 re-ranked with citation signals.
//
//   search_cli <index_dir> <k> <stemmed terms ...> [options]
//
// Options:
//   --cites <w>   weight of the OpenAlex citation count, 0..1   (default 0.05)
//   --graph <w>   weight of citations from inside the corpus     (default 0)
//   --csv <path>  cleaned_papers.csv, needed only when --graph > 0
//   --tsv         machine-readable output (tab separated), used by the web app
//
// The terms must already be stemmed (the Python tokenizer does that), e.g.
//   search_cli data/index 5 reinforc learn
//   search_cli data/index 5 reinforc learn --cites 0.3
//   search_cli data/index 5 reinforc learn --graph 0.1 --csv data/processed/cleaned_papers.csv
// tests/reference_bm25.py prints the stemmed terms for any query.
//
// Weights: BM25 gets (1 - cites - graph). --cites 0 gives pure BM25 order.
// The default 0.05 is a placeholder until it is tuned on judged queries. On the
// 791-paper dataset a weight of 0.15 already changes about 4 of the top 10 papers.

#include "hybrid_ranker.h"
#include "citation_graph.h"
#include "result_ranker.h"
#include "dijkstra.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

static bool parseNumber(const std::string &text, double &value)
{
    char *end = nullptr;
    value = std::strtod(text.c_str(), &end);
    return end != text.c_str() && *end == '\0';
}

int main(int argc, char **argv)
{
    if (argc < 4)
    {
        std::cerr << "Usage: " << argv[0]
                  << " <index_dir> <k> <stemmed terms ...> [--cites w] [--graph w] [--csv path]\n";
        return 1;
    }

    const std::string indexDir = argv[1];
    const int k = std::atoi(argv[2]);

    if (k <= 0)
    {
        std::cerr << "k must be a positive number\n";
        return 1;
    }

    double wCites = 0.05;
    double wGraph = 0.0;
    std::string csvPath;
    bool tsv = false;
    std::vector<std::string> terms;

    for (int i = 3; i < argc; i++)
    {
        const std::string arg = argv[i];

        if (arg == "--tsv")
        {
            tsv = true;
        }
        else if (arg == "--cites" || arg == "--graph" || arg == "--csv")
        {
            if (i + 1 >= argc)
            {
                std::cerr << arg << " needs a value\n";
                return 1;
            }

            const std::string value = argv[++i];

            if (arg == "--csv")
            {
                csvPath = value;
            }
            else
            {
                double number = 0.0;

                if (!parseNumber(value, number))
                {
                    std::cerr << arg << " needs a number, got '" << value << "'\n";
                    return 1;
                }

                (arg == "--cites" ? wCites : wGraph) = number;
            }
        }
        else
        {
            terms.push_back(arg);
        }
    }

    if (terms.empty())
    {
        std::cerr << "Give at least one stemmed term\n";
        return 1;
    }

    if (wCites < 0.0 || wGraph < 0.0 || wCites + wGraph > 1.0)
    {
        std::cerr << "Weights must be >= 0 and --cites + --graph must not exceed 1\n";
        return 1;
    }

    if (wGraph > 0.0 && csvPath.empty())
    {
        std::cerr << "--graph needs --csv <path to cleaned_papers.csv>\n";
        return 1;
    }

    SearchIndex index;
    std::string error;

    if (!loadSearchIndex(indexDir, index, error))
    {
        std::cerr << "Error: " << error << "\n";
        return 1;
    }

    Graph graph;
    bool haveGraph = false;

    if (wGraph > 0.0)
    {
        try
        {
            loadCitationGraph(csvPath, graph);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error loading graph: " << e.what() << "\n";
            return 1;
        }

        if (graph.getVertexCount() != static_cast<int>(index.docs.size()))
        {
            std::cerr << "Warning: the CSV has " << graph.getVertexCount()
                      << " papers but the index has " << index.docs.size()
                      << ". Rebuild the index from the same CSV.\n";
        }

        haveGraph = true;
    }

    const CitationSignals signals = buildCitationSignals(index, haveGraph ? &graph : nullptr);

    std::vector<RankedHit> hits = rankHybrid(index, signals, terms, k, wCites, wGraph);

    if (hits.empty())
    {
        if (!tsv)
            std::cout << "No papers matched.\n";

        return 0;
    }

    // Where did each paper rank under plain BM25? Shows what the citation weight changed.
    std::unordered_map<int, int> bm25Rank;
    std::vector<RankedHit> plain = rankBM25(index, terms, 100);

    for (size_t i = 0; i < plain.size(); i++)
        bm25Rank[plain[i].docIdx] = static_cast<int>(i) + 1;

    if (tsv)
    {
        // rank <TAB> paper_id <TAB> score <TAB> bm25_rank <TAB> citations <TAB> year <TAB> title
        for (size_t i = 0; i < hits.size(); i++)
        {
            const DocInfo &doc = index.docs[hits[i].docIdx];

            std::printf("%zu\t%s\t%.6f\t%d\t%d\t%s\t%s\n",
                        i + 1,
                        doc.paperId.c_str(),
                        hits[i].score,
                        bm25Rank[hits[i].docIdx],
                        doc.citations,
                        doc.year.c_str(),
                        doc.title.c_str());
        }

        return 0;
    }

    std::printf("weights: BM25 %.2f, citations %.2f, graph %.2f\n\n",
                1.0 - wCites - wGraph, wCites, wGraph);

    for (size_t i = 0; i < hits.size(); i++)
    {
        const DocInfo &doc = index.docs[hits[i].docIdx];

        std::printf("%2zu  %6.4f  (BM25 #%d)  [%s, %d cites]  %s\n",
                    i + 1,
                    hits[i].score,
                    bm25Rank[hits[i].docIdx],
                    doc.year.c_str(),
                    doc.citations,
                    doc.title.c_str());
    }

    // Test 2: Unreachable destination

    PathResult unreachable = dijkstra(graph, "D", "A");

    if (unreachable.found)
    {
        cout << "Unreachable test failed\n";
        return 1;
    }

    // Test 3: Source and destination are the same

    PathResult same = dijkstra(graph, "A", "A");

    if (!same.found ||
        same.path.size() != 1 ||
        same.path[0] != "A" ||
        same.totalCost != 0.0)
    {
        cout << "Same vertex test failed\n";
        return 1;
    }

    cout << "ALL DIJKSTRA TESTS PASSED\n";
    return 0;
}