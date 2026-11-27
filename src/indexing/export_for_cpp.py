"""Export the search index as plain text files that the C++ engine can read.

Put this file at  src/indexing/export_for_cpp.py  and run from the repo root:
    python src/indexing/export_for_cpp.py

Reads : data/processed/cleaned_papers.csv
Writes: data/index/docs.tsv      one line per paper
        data/index/postings.tsv  one line per term
        data/index/edges.tsv     one line per in-corpus citation

File formats (tab separated, UTF-8, no header line):

  docs.tsv      doc_idx  paper_id  weighted_len  citations  year  title
  postings.tsv  term  df  doc_idx:tf  doc_idx:tf ...      (doc_idx ascending)
  edges.tsv     citing_idx  cited_idx                      (A cites B -> A B)

doc_idx is the 0-based row number in cleaned_papers.csv.

Field weights match Member C's rank_results(): title x5, topics x2, abstract x1.
tf is the weighted term count and weighted_len is the weighted token count, so
BM25 length normalisation uses the same weighted numbers.
"""
import argparse
import csv
import sys
from collections import Counter
from pathlib import Path

DEFAULT_ROOT = Path(__file__).resolve().parents[2]

FIELDS = (("title", 5), ("topics", 2), ("abstract", 1))


def one_line(text):
    """Collapse tabs and newlines so a value never breaks the TSV layout."""
    return " ".join((text or "").split())


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", default=str(DEFAULT_ROOT),
                    help="repository root (default: derived from this file's location)")
    args = ap.parse_args()

    root = Path(args.root).resolve()
    sys.path.insert(0, str(root))
    from src.preprocessing.tokenizer import tokenize  # same tokenizer as the Python engine

    csv.field_size_limit(sys.maxsize)

    src_file = root / "data" / "processed" / "cleaned_papers.csv"
    out_dir = root / "data" / "index"
    out_dir.mkdir(parents=True, exist_ok=True)

    with open(src_file, encoding="utf-8", newline="") as f:
        papers = list(csv.DictReader(f))

    id_to_idx = {p["paper_id"].strip(): i for i, p in enumerate(papers)}

    postings = {}          # term -> [(doc_idx, weighted_tf), ...]
    doc_lines = []
    total_len = 0

    for i, p in enumerate(papers):
        tf = Counter()
        length = 0
        for field, weight in FIELDS:
            tokens = tokenize(p.get(field, "") or "")
            length += weight * len(tokens)
            for t in tokens:
                tf[t] += weight
        for term, count in tf.items():
            postings.setdefault(term, []).append((i, count))
        total_len += length

        try:
            citations = int(p.get("citations") or 0)
        except ValueError:
            citations = 0
        doc_lines.append("\t".join([
            str(i),
            p["paper_id"].strip(),
            str(length),
            str(citations),
            one_line(p.get("year", "")),
            one_line(p.get("title", "")),
        ]))

    edges = set()
    for i, p in enumerate(papers):
        for ref in p.get("references", "").split(";"):
            j = id_to_idx.get(ref.strip())
            if j is not None and j != i:
                edges.add((i, j))

    with open(out_dir / "docs.tsv", "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(doc_lines) + "\n")

    with open(out_dir / "postings.tsv", "w", encoding="utf-8", newline="\n") as f:
        for term in sorted(postings):
            plist = postings[term]          # already in ascending doc_idx order
            cells = " ".join(f"{d}:{c}" for d, c in plist)
            f.write(f"{term}\t{len(plist)}\t{cells}\n")

    with open(out_dir / "edges.tsv", "w", encoding="utf-8", newline="\n") as f:
        for a, b in sorted(edges):
            f.write(f"{a}\t{b}\n")

    n = len(papers)
    print("Exported to:", out_dir)
    print("  documents      :", n)
    print("  terms          :", len(postings))
    print("  posting entries:", sum(len(v) for v in postings.values()))
    print("  citation edges :", len(edges))
    print(f"  avg weighted doc length: {total_len / n:.2f}" if n else "  (no documents)")


if __name__ == "__main__":
    main()