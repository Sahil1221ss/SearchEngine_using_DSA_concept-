import csv
from collections import Counter


FILE = "data/processed/papers.csv"


papers = []

with open(FILE, "r", encoding="utf-8") as file:

    reader = csv.DictReader(file)

    for row in reader:
        papers.append(row)


print("===================================")
print("DATASET QUALITY REPORT")
print("===================================")

print("Total papers:", len(papers))


missing_titles = 0
missing_abstracts = 0
missing_authors = 0
missing_years = 0

titles = []
years = []
abstract_lengths = []


for paper in papers:

    title = paper["title"].strip()
    abstract = paper["abstract"].strip()
    authors = paper["authors"].strip()
    year = paper["year"].strip()

    if not title:
        missing_titles += 1
    else:
        titles.append(title.lower())

    if not abstract:
        missing_abstracts += 1
    else:
        abstract_lengths.append(len(abstract.split()))

    if not authors:
        missing_authors += 1

    if not year:
        missing_years += 1
    else:
        years.append(int(year))


title_counts = Counter(titles)

duplicate_titles = sum(
    1 for count in title_counts.values()
    if count > 1
)


print("\nMissing data")
print("----------------------------")
print("Missing titles:", missing_titles)
print("Missing abstracts:", missing_abstracts)
print("Missing authors:", missing_authors)
print("Missing years:", missing_years)


print("\nDuplicates")
print("----------------------------")
print("Duplicate titles:", duplicate_titles)


if years:

    print("\nPublication years")
    print("----------------------------")
    print("Oldest paper:", min(years))
    print("Newest paper:", max(years))


if abstract_lengths:

    average_length = sum(abstract_lengths) / len(abstract_lengths)

    print("\nAbstract information")
    print("----------------------------")
    print("Papers with abstracts:", len(abstract_lengths))
    print("Average abstract words:", round(average_length, 2))


print("\n===================================")
print("Report completed")
print("===================================")