import json
import csv
import re


INDEX_FILE = "data/index/inverted_index.json"
PAPERS_FILE = "data/processed/cleaned_papers.csv"


def tokenize(text):
    text = text.lower()
    return re.findall(r"\b[a-zA-Z0-9]+\b", text)


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


index = load_index()
papers = load_papers()


print("==========================================")
print("       ACADEMIC SEARCH ENGINE")
print("==========================================")

query = input("\nEnter search query: ")

results = search(query, index)

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