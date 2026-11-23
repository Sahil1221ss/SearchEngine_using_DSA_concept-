import json
import csv
import sys
from pathlib import Path
import math
ROOT_DIR = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT_DIR))

from src.preprocessing.tokenizer import tokenize


INDEX_FILE = "data/index/inverted_index.json"
PAPERS_FILE = "data/processed/cleaned_papers.csv"


def load_index():
    with open(INDEX_FILE, "r", encoding="utf-8") as file:
        return json.load(file)


def load_papers():

    papers = {}

    with open(PAPERS_FILE, "r", encoding="utf-8") as file:

        reader = csv.DictReader(file)

        for paper in reader:
            papers[paper["paper_id"]] = paper

    return papers


def search(query, index):

    words = tokenize(query)

    if not words:
        return []

    result_sets = []

    for word in words:

        if word in index:
            result_sets.append(set(index[word]))
        else:
            return []

    results = result_sets[0]

    for result in result_sets[1:]:
        results = results.intersection(result)

    return list(results)


def rank_results(results, query, papers, index):

    query_words = tokenize(query)
    total_documents = len(papers)

    ranked_results = []

    for paper_id in results:

        paper = papers.get(paper_id)

        if not paper:
            continue

        title_words = tokenize(paper.get("title", ""))
        abstract_words = tokenize(paper.get("abstract", ""))
        topic_words = tokenize(paper.get("topics", ""))

        score = 0

        for word in query_words:

            # Inverse Document Frequency (IDF)
            document_frequency = len(index.get(word, []))

            idf = math.log(
                (total_documents + 1) / (document_frequency + 1)
            ) + 1

            # Weighted Term Frequency (TF)
            term_frequency = (
                5 * title_words.count(word)
                + 2 * topic_words.count(word)
                + abstract_words.count(word)
            )

            # TF-IDF score
            score += term_frequency * idf

        # Small citation bonus
        try:
            citations = int(paper.get("citations", 0))
        except (ValueError, TypeError):
            citations = 0

        score += min(math.log1p(max(0, citations)), 10) * 0.1

        ranked_results.append((paper_id, score))

    # Highest score first
    ranked_results.sort(
        key=lambda item: (
            -item[1],
            papers[item[0]].get("title", "").lower()
        )
    )

    return [paper_id for paper_id, score in ranked_results]

def main():

    index = load_index()
    papers = load_papers()

    print("==========================================")
    print("       ACADEMIC SEARCH ENGINE")
    print("==========================================")

    query = input("\nEnter search query: ")
    results = rank_results(search(query, index), query, papers, index)
    print("\nSearch results:", len(results))
    print("------------------------------------------")

    if not results:
        print("No papers found.")

    else:

        for i, paper_id in enumerate(results[:10], 1):

            paper = papers.get(paper_id)

            if paper:
                print(f"\n{i}. {paper['title']}")
                print("   Year:", paper["year"])
                print("   Authors:", paper["authors"])
                print("   Citations:", paper["citations"])


if __name__ == "__main__":
    main()
