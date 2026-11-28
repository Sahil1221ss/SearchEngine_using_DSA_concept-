
import csv
import subprocess
from pathlib import Path

from flask import Flask, render_template, request

from src.preprocessing.tokenizer import tokenize


app = Flask(__name__)

ROOT_DIR = Path(__file__).resolve().parents[2]

SEARCH_EXECUTABLE = ROOT_DIR / "build" / "search_cli.exe"
CITATION_EXECUTABLE = ROOT_DIR / "build" / "citation_path_cli.exe"

INDEX_DIR = ROOT_DIR / "data" / "index"
CSV_PATH = ROOT_DIR / "data" / "processed" / "cleaned_papers.csv"

papers = {}

with open(CSV_PATH, "r", encoding="utf-8", newline="") as file:
    reader = csv.DictReader(file)

    for paper in reader:
        papers[paper["paper_id"]] = paper

paper_options = sorted(
    [
        {
            "paper_id": paper["paper_id"],
            "label": (
                f'{paper["title"]} ({paper["year"]})'
                f' — {paper["paper_id"]}'
            )
        }
        for paper in papers.values()
    ],
    key=lambda paper: paper["label"].lower()
)

def get_selected_paper_id(selection):

    if " — " not in selection:
        return None

    paper_id = selection.rsplit(" — ", 1)[-1].strip()

    if paper_id not in papers:
        return None

    return paper_id

def find_citation_path(source_id, destination_id):

    command = [
        str(CITATION_EXECUTABLE),
        str(CSV_PATH),
        source_id,
        destination_id
    ]

    process = subprocess.run(
        command,
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace",
        check=True
    )

    lines = process.stdout.strip().splitlines()

    if not lines:
        raise ValueError("The citation program returned no output.")

    if lines[0] == "NOT_FOUND":
        return None

    header = lines[0].split("\t")

    if len(header) != 2 or header[0] != "FOUND":
        raise ValueError("Unexpected output from the citation program.")

    total_cost = float(header[1])

    if len(lines) < 2:
        raise ValueError("The citation path was missing from the output.")

    path_parts = lines[1].split("\t")

    if path_parts[0] != "PATH":
        raise ValueError("Unexpected citation path format.")

    path_papers = []

    for paper_id in path_parts[1:]:
        paper = papers.get(paper_id)

        if paper:
            path_papers.append(paper)

    return {
        "total_cost": total_cost,
        "papers": path_papers
    }

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
    encoding="utf-8",
    errors="replace",
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
        error=error,
        paper_options=paper_options,
        citation_path=None,
        citation_error=None,
        selected_source="",
        selected_destination=""
    )

@app.route("/citation-path", methods=["POST"])
def citation_path():

    selected_source = request.form.get("source", "").strip()
    selected_destination = request.form.get("destination", "").strip()

    path_result = None
    citation_error = None

    source_id = get_selected_paper_id(selected_source)
    destination_id = get_selected_paper_id(selected_destination)

    if not source_id or not destination_id:
        citation_error = ("Please pick both papers from the suggestion list. Type a few words of the title, then click one of the entries (each ends with the paper ID).")

    else:
        try:
            path_result = find_citation_path(
                source_id,
                destination_id
            )

            if path_result is None:
                citation_error = (
                    "No citation route leads from the source to the destination. "
                    "Citations only point one way, from a paper to the earlier papers it cites, "
                    "so try swapping the two papers."
                )

        except FileNotFoundError:
            citation_error = "The citation-path executable was not found."

        except subprocess.CalledProcessError as e:
            citation_error = (
                "C++ citation-path program failed: "
                + (e.stderr or str(e))
            )

        except Exception as e:
            citation_error = "An error occurred: " + str(e)

    return render_template(
        "index.html",
        results=[],
        query="",
        error=None,
        paper_options=paper_options,
        citation_path=path_result,
        citation_error=citation_error,
        selected_source=selected_source,
        selected_destination=selected_destination
    )

if __name__ == "__main__":
    app.run(debug=True)