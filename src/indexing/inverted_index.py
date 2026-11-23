

import csv
import json
import os
import sys
from pathlib import Path

ROOT_DIR = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT_DIR))

from src.preprocessing.tokenizer import tokenize


INPUT_FILE = "data/processed/cleaned_papers.csv"
OUTPUT_FILE = "data/index/inverted_index.json"


def build_index():

    index = {}

    # Read the cleaned research papers
    with open(INPUT_FILE, "r", encoding="utf-8") as file:

        reader = csv.DictReader(file)

        for paper in reader:

            paper_id = paper["paper_id"]
            text = paper["search_text"]

            # Use our improved tokenizer
            words = tokenize(text)

            for word in words:

                if word not in index:
                    index[word] = []

                if paper_id not in index[word]:
                    index[word].append(paper_id)

    return index


def save_index(index):

    os.makedirs("data/index", exist_ok=True)

    with open(OUTPUT_FILE, "w", encoding="utf-8") as file:

        json.dump(index, file, indent=4)

    print("===================================")
    print("INVERTED INDEX CREATED")
    print("===================================")

    print("Unique words:", len(index))
    print("Saved to:", OUTPUT_FILE)


if __name__ == "__main__":

    index = build_index()
    save_index(index)
