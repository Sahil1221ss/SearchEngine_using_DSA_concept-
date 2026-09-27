# System Design

## Architecture Overview
- **Data Collection**: Fetch paper metadata from OpenAlex API.
- **Preprocessing**: Clean, tokenize, and filter text.
- **Indexing**: Inverted index and Trie data structures for fast lookups.
- **Search & Ranking**: Query processing, TF-IDF / citation-based ranking.
- **Citation Graph**: Graph algorithms (BFS, DFS) for citation traversal and metrics.