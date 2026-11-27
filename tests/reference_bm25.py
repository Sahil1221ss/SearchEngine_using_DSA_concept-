"""Reference BM25 in Python: the answer key the C++ ranker must match.

Put this file at  tests/reference_bm25.py  and run from the repo root:
    python tests/reference_bm25.py "deep learning" "neural machine translation"
    python tests/reference_bm25.py --tsv --out tests/golden_bm25.tsv "deep learning" "data mining"

Use --out instead of the shell's ">" : Windows PowerShell's ">" writes UTF-16, which
the C++ test cannot read.

Reads the files written by src/indexing/export_for_cpp.py.

BM25:  score(d) = sum over distinct query terms t of
           idf(t) * tf * (k1 + 1) / ( tf + k1 * (1 - b + b * len(d) / avglen) )
       idf(t) = ln(1 + (N - df + 0.5) / (df + 0.5))        k1 = 1.2, b = 0.75

Every paper containing at least ONE query term is scored (OR semantics).

Ties (exactly equal scores are common: papers with the same title and no
abstract score identically) are broken by LARGER paper_id first. This matches
MinHeap::isSmaller() in src/dsa/priority_queue.cpp, where the smaller paper_id
is the one evicted first. If you ever flip the heap's tie rule, flip the
`reverse=True` below and regenerate tests/golden_bm25.tsv.

Terms are added in sorted order so the floating point sums are identical on
every run and match the C++ ranker.
"""
import argparse
import math
import sys
from pathlib import Path

DEFAULT_ROOT = Path(__file__).resolve().parents[1]
K1 = 1.2
B = 0.75


def load(root):
    idx = Path(root) / "data" / "index"
    docs = []
    with open(idx / "docs.tsv", encoding="utf-8") as f:
        for line in f:
            i, pid, length, cites, year, title = line.rstrip("\n").split("\t", 5)
            docs.append({"paper_id": pid, "len": int(length), "cites": int(cites),
                         "year": year, "title": title})
    postings = {}
    with open(idx / "postings.tsv", encoding="utf-8") as f:
        for line in f:
            term, df, cells = line.rstrip("\n").split("\t")
            plist = []
            for cell in cells.split(" "):
                d, c = cell.split(":")
                plist.append((int(d), int(c)))
            postings[term] = plist
    return docs, postings


def bm25_top_k(terms, docs, postings, k=10):
    n = len(docs)
    avglen = sum(d["len"] for d in docs) / n
    scores = {}
    for t in sorted(set(terms)):
        plist = postings.get(t)
        if not plist:
            continue
        df = len(plist)
        idf = math.log(1 + (n - df + 0.5) / (df + 0.5))
        for idx, tf in plist:
            norm = tf + K1 * (1 - B + B * docs[idx]["len"] / avglen)
            scores[idx] = scores.get(idx, 0.0) + idf * tf * (K1 + 1) / norm
    # two stable passes: first paper_id descending, then score descending
    ranked = sorted(scores.items(), key=lambda kv: docs[kv[0]]["paper_id"], reverse=True)
    ranked.sort(key=lambda kv: -kv[1])
    return ranked[:k], len(scores)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("queries", nargs="+")
    ap.add_argument("--root", default=str(DEFAULT_ROOT))
    ap.add_argument("--k", type=int, default=10)
    ap.add_argument("--tsv", action="store_true", help="machine-readable output")
    ap.add_argument("--out", help="with --tsv: write to this file (UTF-8, LF line ends) instead of the screen")
    args = ap.parse_args()

    root = Path(args.root).resolve()
    sys.path.insert(0, str(root))
    from src.preprocessing.tokenizer import tokenize

    docs, postings = load(root)
    tsv_lines = []

    for q in args.queries:
        terms = tokenize(q)
        top, matched = bm25_top_k(terms, docs, postings, args.k)
        if args.tsv:
            # query <TAB> stemmed terms <TAB> rank <TAB> paper_id <TAB> score
            for rank, (idx, score) in enumerate(top, 1):
                tsv_lines.append(f"{q}\t{' '.join(terms)}\t{rank}\t{docs[idx]['paper_id']}\t{score:.6f}")
        else:
            print(f'\nQuery: "{q}"  ->  stemmed terms {terms}  |  {matched} papers match at least one term')
            for rank, (idx, score) in enumerate(top, 1):
                d = docs[idx]
                print(f"  {rank:2d}. {score:8.4f}  [{d['year']}, {d['cites']} cites]  {d['title'][:80]}")

    if args.tsv:
        text = "".join(line + "\n" for line in tsv_lines)
        if args.out:
            with open(args.out, "w", encoding="utf-8", newline="\n") as f:
                f.write(text)
            print(f"Wrote {len(tsv_lines)} lines to {args.out}")
        else:
            sys.stdout.write(text)


if __name__ == "__main__":
    main()