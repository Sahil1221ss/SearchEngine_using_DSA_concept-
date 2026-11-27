
import csv
import subprocess
from pathlib import Path

from flask import Flask, render_template, request

from src.preprocessing.tokenizer import tokenize


app = Flask(__name__)

ROOT_DIR = Path(__file__).resolve().parents[2]

SEARCH_EXECUTABLE = ROOT_DIR / "build" / "search_cli.exe"
INDEX_DIR = ROOT_DIR / "data" / "index"
CSV_PATH = ROOT_DIR / "data" / "processed" / "cleaned_papers.csv"

papers = {}

with open(CSV_PATH, "r", encoding="utf-8", newline="") as file:
    reader = csv.DictReader(file)

    for paper in reader:
        papers[paper["paper_id"]] = paper


def search_papers(query):

    terms = tokenize(query)

    if not terms:
        return []

    command = [
        str(SEARCH_EXECUTABLE),
        str(INDEX_DIR),
        "50",
        *terms,
        "--cites", "0.15",
        "--graph", "0.10",
        "--csv", str(CSV_PATH),
        "--tsv"
    ]

    process = subprocess.run(
        command,
        capture_output=True,
        text=True,
        check=True
    )

    results = []

    for line in process.stdout.splitlines():

        parts = line.split("\t", 6)

        if len(parts) != 7:
            continue

        rank, paper_id, score, bm25_rank, citations, year, title = parts

        paper = papers.get(paper_id)

        if paper:
            results.append({
                **paper,
                "rank": rank,
                "score": score,
                "bm25_rank": bm25_rank,
                "title": title
            })

    return results


@app.route("/", methods=["GET", "POST"])
def home():

    results = []
    query = ""
    error = None

    if request.method == "POST":

        query = request.form.get("query", "").strip()

        if query:
            try:
                results = search_papers(query)

            except FileNotFoundError:
                error = "Search executable or required data file was not found."

            except subprocess.CalledProcessError as e:
                error = "C++ search failed: " + (e.stderr or str(e))

            except Exception as e:
                error = "An error occurred: " + str(e)

    return render_template(
        "index.html",
        results=results,
        query=query,
        error=error
    )


if __name__ == "__main__":
    app.run(debug=True)
