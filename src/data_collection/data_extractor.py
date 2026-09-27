import json
import csv
import os


INPUT_FILE = "data/raw/openalex_raw.json"
OUTPUT_FILE = "data/processed/papers.csv"


def reconstruct_abstract(abstract_index):
    if not abstract_index:
        return ""

    words = []

    for word, positions in abstract_index.items():
        for position in positions:
            words.append((position, word))

    words.sort()

    return " ".join(word for position, word in words)


def extract_authors(authorships):
    authors = []

    for author in authorships:
        author_name = author.get("author", {}).get("display_name")

        if author_name:
            authors.append(author_name)

    return "; ".join(authors)


def extract_topics(topics):
    topic_names = []

    for topic in topics:
        name = topic.get("display_name")

        if name:
            topic_names.append(name)

    return "; ".join(topic_names)


def extract_references(references):
    return "; ".join(references)


with open(INPUT_FILE, "r", encoding="utf-8") as file:
    data = json.load(file)


os.makedirs("data/processed", exist_ok=True)


papers = []

for paper in data["results"]:

    paper_id = paper.get("id", "")

    title = paper.get("title", "")

    abstract = reconstruct_abstract(
        paper.get("abstract_inverted_index")
    )

    authors = extract_authors(
        paper.get("authorships", [])
    )

    year = paper.get("publication_year", "")

    topics = extract_topics(
        paper.get("topics", [])
    )

    citations = paper.get("cited_by_count", 0)

    doi = paper.get("doi", "")

    references = extract_references(
        paper.get("referenced_works", [])
    )

    papers.append([
        paper_id,
        title,
        abstract,
        authors,
        year,
        topics,
        citations,
        doi,
        references
    ])


with open(
    OUTPUT_FILE,
    "w",
    newline="",
    encoding="utf-8"
) as file:

    writer = csv.writer(file)

    writer.writerow([
        "paper_id",
        "title",
        "abstract",
        "authors",
        "year",
        "topics",
        "citations",
        "doi",
        "references"
    ])

    writer.writerows(papers)


print("Data extraction completed.")
print("Total papers:", len(papers))
print("CSV created:", OUTPUT_FILE)