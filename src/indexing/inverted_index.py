import csv
import re
import json
import os


INPUT_FILE = "data/processed/cleaned_papers.csv"
OUTPUT_FILE = "data/index/inverted_index.json"


def tokenize(text):

    text = text.lower()

    words = re.findall(r"\b[a-zA-Z0-9]+\b", text)

    return words


index = {}


with open(INPUT_FILE, "r", encoding="utf-8") as file:

    reader = csv.DictReader(file)

    for paper in reader:

        paper_id = paper["paper_id"]

        text = paper["search_text"]

        words = tokenize(text)

        for word in words:

            if word not in index:
                index[word] = []

            if paper_id not in index[word]:
                index[word].append(paper_id)


os.makedirs("data/index", exist_ok=True)


with open(
    OUTPUT_FILE,
    "w",
    encoding="utf-8"
) as file:

    json.dump(
        index,
        file,
        indent=4
    )


print("===================================")
print("INVERTED INDEX CREATED")
print("===================================")

print("Unique words:", len(index))

print("Saved to:", OUTPUT_FILE)