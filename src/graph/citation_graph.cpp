
#include "citation_graph.h"
#include <cmath>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <cctype>

// Removes leading and trailing whitespace
static std::string trim(const std::string &str)
{
    size_t start = 0;
    size_t end = str.size();

    while (start < end && isspace(static_cast<unsigned char>(str[start])))
        start++;

    while (end > start && isspace(static_cast<unsigned char>(str[end - 1])))
        end--;

    return str.substr(start, end - start);
}

// Reads one complete CSV record, including quoted multiline fields
static bool readCSVRecord(
    std::istream &file,
    std::vector<std::string> &row)
{
    row.clear();

    std::string field;
    bool insideQuotes = false;
    bool recordStarted = false;

    char ch;

    while (file.get(ch))
    {
        recordStarted = true;

        if (insideQuotes)
        {
            if (ch == '"')
            {
                if (file.peek() == '"')
                {
                    file.get(ch);
                    field += '"';
                }
                else
                {
                    insideQuotes = false;
                }
            }
            else
            {
                field += ch;
            }
        }
        else
        {
            if (ch == '"')
            {
                insideQuotes = true;
            }
            else if (ch == ',')
            {
                row.push_back(field);
                field.clear();
            }
            else if (ch == '\n')
            {
                row.push_back(field);
                return true;
            }
            else if (ch != '\r')
            {
                field += ch;
            }
        }
    }

    if (!recordStarted)
        return false;

    row.push_back(field);
    return true;
}

// Splits semicolon-separated reference IDs
static std::vector<std::string> splitReferences(
    const std::string &references)
{
    std::vector<std::string> result;

    std::stringstream ss(references);
    std::string reference;

    while (std::getline(ss, reference, ';'))
    {
        reference = trim(reference);

        if (!reference.empty())
            result.push_back(reference);
    }

    return result;
}

CitationGraphStats loadCitationGraph(
    const std::string &csvPath,
    Graph &graph)
{
    std::ifstream file(csvPath);

    if (!file.is_open())
        throw std::runtime_error("Could not open CSV file");

    CitationGraphStats stats;

    std::vector<std::string> row;
    std::unordered_map<std::string, size_t> column;

    // Read header
    if (!readCSVRecord(file, row))
        throw std::runtime_error("CSV file is empty");

    for (size_t i = 0; i < row.size(); i++)
        column[trim(row[i])] = i;

    if (column.find("paper_id") == column.end() ||
        column.find("references") == column.end())
    {
        throw std::runtime_error(
            "CSV must contain paper_id and references columns");
    }

    size_t idIndex = column["paper_id"];
    size_t refsIndex = column["references"];

    std::vector<std::pair<std::string, std::string>> papers;

    // First pass: collect papers and create vertices
    while (readCSVRecord(file, row))
    {
        if (row.size() <= idIndex || row.size() <= refsIndex)
            continue;

        std::string paperID = trim(row[idIndex]);
        std::string references = trim(row[refsIndex]);

        if (paperID.empty())
            continue;

        if (!graph.hasVertex(paperID))
        {
            graph.addVertex(paperID);
            stats.totalPapers++;
        }

        papers.push_back({paperID, references});
    }

    // Second pass: calculate incoming citation counts
    unordered_map<string, size_t> incoming;

    for (const auto &paper : papers)
        incoming[paper.first] = 0;

    for (const auto &paper : papers)
    {
        vector<string> references = splitReferences(paper.second);

        if (references.empty())
            stats.papersWithNoReferences++;

        for (const string &reference : references)
        {
            stats.totalReferenceLinks++;

            if (graph.hasVertex(reference))
            {
                incoming[reference]++;
                stats.internalReferenceLinks++;
            }
        }
    }

    // Third pass: create weighted citation edges
    for (const auto &paper : papers)
    {
        vector<string> references = splitReferences(paper.second);

        for (const string &reference : references)
        {
            if (graph.hasVertex(reference))
            {
                double citationCount =
                    static_cast<double>(incoming[reference]);

                double weight =
                    1.0 / (1.0 + log1p(citationCount));

                graph.addWeightedEdge(
                    paper.first,
                    reference,
                    weight);
            }
        }
    }

    // Calculate citation statistics
    for (const auto &entry : incoming)
    {
        if (entry.second > 0)
            stats.papersCitedAtLeastOnce++;

        if (entry.second > stats.maxInternalCitations)
            stats.maxInternalCitations = entry.second;
    }

    if (stats.totalPapers > 0)
    {
        stats.averageInternalReferences =
            static_cast<double>(stats.internalReferenceLinks) /
            stats.totalPapers;
    }

    return stats;
}
