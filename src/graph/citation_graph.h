
#ifndef CITATION_GRAPH_H
#define CITATION_GRAPH_H

#include "../dsa/graph.h"
#include <string>
#include <cstddef>

struct CitationGraphStats
{
    size_t totalPapers = 0;
    size_t papersWithNoReferences = 0;
    size_t totalReferenceLinks = 0;
    size_t internalReferenceLinks = 0;
    size_t papersCitedAtLeastOnce = 0;
    size_t maxInternalCitations = 0;
    double averageInternalReferences = 0.0;
};

CitationGraphStats loadCitationGraph(
    const std::string &csvPath,
    Graph &graph);

#endif
