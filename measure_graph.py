"""Measure how connected the citation graph is INSIDE the corpus.

Run from the repository root:
    python measure_graph.py
"""
import csv
import sys

csv.field_size_limit(2147483647)

PATH = "data/processed/cleaned_papers.csv"

rows = []
ids = set()
with open(PATH, encoding="utf-8", newline="") as f:
    for r in csv.DictReader(f):
        rows.append(r)
        ids.add(r["paper_id"].strip())

no_refs = 0
total_links = 0
inside_links = 0
in_degree = {}

for r in rows:
    refs = [x.strip() for x in r["references"].split(";") if x.strip()]
    if not refs:
        no_refs += 1
    for ref in refs:
        total_links += 1
        if ref in ids:
            inside_links += 1
            in_degree[ref] = in_degree.get(ref, 0) + 1

n = len(rows)
pct = (100.0 * inside_links / total_links) if total_links else 0.0
avg_inside = inside_links / n if n else 0.0

print("=== Citation graph measurement ===")
print("Papers in corpus               :", n)
print("Papers with no references      :", no_refs)
print("Total reference links          :", total_links)
print("Links pointing inside corpus   :", inside_links, f"({pct:.1f}%)")
print(f"Avg in-corpus refs per paper   : {avg_inside:.2f}")
print("Papers cited at least once     :", len(in_degree))
print("Max in-corpus citations (paper):", max(in_degree.values()) if in_degree else 0)

print("\n=== Sample rows (for checking the format) ===")
for r in rows[:2]:
    refs = [x.strip() for x in r["references"].split(";") if x.strip()]
    print("id        :", r["paper_id"])
    print("title     :", r["title"][:70])
    print("citations :", r["citations"])
    print("refs      :", len(refs), "| first:", refs[0] if refs else "-")
    print("abstract  :", len(r["abstract"]), "characters")
    print()




import csv

with open("data/processed/cleaned_papers.csv", "r", encoding="utf-8-sig", newline="") as file:

    reader = csv.DictReader(file)

    print("COLUMNS:")
    print(reader.fieldnames)

    print("\nFIRST RECORD:")

    row = next(reader)

    for key, value in row.items():
        print(f"{key}: {repr(value)}")