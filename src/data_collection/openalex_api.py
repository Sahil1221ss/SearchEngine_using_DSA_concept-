import requests
import json
import os
import time


URL = "https://api.openalex.org/works"

topics = [
    "machine learning",
    "deep learning",
    "artificial intelligence",
    "computer vision",
    "natural language processing",
    "data science",
    "data mining",
    "generative artificial intelligence"
]

TARGET_PAPERS = 5000
PAPERS_PER_REQUEST = 100

papers = {}


for topic in topics:

    print("\nCollecting:", topic)

    cursor = "*"

    while len(papers) < TARGET_PAPERS:

        params = {
            "search": topic,
            "per-page": PAPERS_PER_REQUEST,
            "cursor": cursor
        }

        response = requests.get(URL, params=params)

        if response.status_code != 200:
            print("Error:", response.status_code)
            break

        data = response.json()

        results = data["results"]

        if not results:
            break

        for paper in results:

            paper_id = paper.get("id")

            if paper_id:
                papers[paper_id] = paper

        print(
            "Topic:", topic,
            "| Unique papers:", len(papers)
        )

        cursor = data["meta"]["next_cursor"]

        if not cursor:
            break

        time.sleep(0.2)

    if len(papers) >= TARGET_PAPERS:
        break


all_papers = list(papers.values())

all_papers = all_papers[:TARGET_PAPERS]


os.makedirs("data/raw", exist_ok=True)


with open(
    "data/raw/openalex_raw.json",
    "w",
    encoding="utf-8"
) as file:

    json.dump(
        {
            "results": all_papers
        },
        file,
        indent=4
    )


print("\n==============================")
print("Data collection completed")
print("==============================")
print("Unique papers:", len(all_papers))
print("Saved to: data/raw/openalex_raw.json")