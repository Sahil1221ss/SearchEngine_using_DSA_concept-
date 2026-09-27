import csv
import os


INPUT_FILE = "data/processed/papers.csv"
OUTPUT_FILE = "data/processed/cleaned_papers.csv"


papers = []

with open(INPUT_FILE, "r", encoding="utf-8") as file:

    reader = csv.DictReader(file)

    for row in reader:
        papers.append(row)


cleaned_papers = []
seen_ids = set()


for paper in papers:

    paper_id = paper["paper_id"].strip()
    title = paper["title"].strip()
    abstract = paper["abstract"].strip()
    authors = paper["authors"].strip()
    year = paper["year"].strip()
    topics = paper["topics"].strip()
    citations = paper["citations"].strip()
    doi = paper["doi"].strip()
    references = paper["references"].strip()

    if not paper_id:
        continue

    if paper_id in seen_ids:
        continue

    if not title:
        continue

    seen_ids.add(paper_id)

    search_text = " ".join([
        title,
        abstract,
        topics
    ]).strip()

    cleaned_papers.append([
        paper_id,
        title,
        abstract,
        authors,
        year,
        topics,
        citations,
        doi,
        references,
        search_text
    ])


os.makedirs("data/processed", exist_ok=True)


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
        "references",
        "search_text"
    ])

    writer.writerows(cleaned_papers)


print("Cleaning completed.")
print("Original papers:", len(papers))
print("Cleaned papers:", len(cleaned_papers))
print("Removed papers:", len(papers) - len(cleaned_papers))
print("Created:", OUTPUT_FILE)