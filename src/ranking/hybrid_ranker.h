#ifndef HYBRID_RANKER_H
#define HYBRID_RANKER_H

#include "result_ranker.h"

#include <string>
#include <vector>

class Graph; // defined in src/dsa/graph.h

// One 0..1 "importance" number per paper (indexed by docIdx).
struct CitationSignals
{
    // log(1 + OpenAlex cited-by count), min-max scaled to 0..1
    std::vector<double> citations;

    // log(1 + number of papers INSIDE the corpus that cite this one), scaled to 0..1.
    // All zero when no graph is given.
    std::vector<double> graph;
};

// Builds both signals once. Pass nullptr for graph to skip the in-corpus signal.
CitationSignals buildCitationSignals(const SearchIndex &index, const Graph *graph);

// Re-ranks the best `candidates` BM25 hits and returns the top k:
//
//   final = (1 - wCites - wGraph) * (bm25 / best bm25)
//           + wCites * signals.citations
//           + wGraph * signals.graph
//
// Only the BM25 candidates are re-ranked, so a famous but off-topic paper can never
// jump in from outside the pool. Throws std::invalid_argument if a weight is negative,
// wCites + wGraph > 1, or the signals do not match the index.
std::vector<RankedHit> rankHybrid(const SearchIndex &index,
                                  const CitationSignals &signals,
                                  const std::vector<std::string> &stemmedTerms,
                                  int k,
                                  double wCites,
                                  double wGraph,
                                  int candidates = 100);

#endif