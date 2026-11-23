
from flask import Flask, render_template, request

from src.search.search_engine import (
    load_index,
    load_papers,
    search,
    rank_results
)

app = Flask(__name__)

# Load the search data
index = load_index()
papers = load_papers()


@app.route("/", methods=["GET", "POST"])
def home():

    results = []
    query = ""

    if request.method == "POST":

        query = request.form.get("query", "").strip()

        if query:
            paper_ids = search(query, index)
            candidate_ids = paper_ids[:500]
            ranked_ids = rank_results(
                candidate_ids,
                query,
                papers,
                index
            )


            for paper_id in ranked_ids[:50]:
                paper = papers.get(paper_id)

                if paper:
                    results.append(paper)

    return render_template(
        "index.html",
        results=results,
        query=query
    )


if __name__ == "__main__":
    app.run(debug=True)
