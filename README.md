# Academic Search Engine using DSA

A DSA-based academic search engine project that collects research-paper data, preprocesses the data, and builds an inverted index for efficient searching.

The project is being developed as a **Data Structures and Algorithms (DSA) capstone project**.

---

## 📌 Current Project Status

The current version of the project implements the initial data pipeline:

```text
OpenAlex API
      ↓
Raw Academic Data
      ↓
Data Extraction
      ↓
Data Quality Check
      ↓
Data Cleaning
      ↓
Inverted Index
```

The search, ranking, graph analysis, and additional DSA components are currently under development.

---

## 🎯 Project Objective

The main objective is to build an academic search engine that demonstrates how different Data Structures and Algorithms can be applied to a real-world search system.

The project will eventually include concepts such as:

* Inverted Index
* Hash Table
* Trie
* Searching
* Sorting
* Priority Queue
* Linked List
* Graph
* BFS
* DFS
* Ranking

---

## 📚 Data Source

The project uses **OpenAlex** as the source of academic research-paper metadata.

The project collects information such as:

* Paper title
* Authors
* Publication year
* Abstract
* Paper ID
* Other available academic metadata

The dataset is generated locally using the data-collection scripts.

### Important

The generated dataset is **not stored in this GitHub repository**.

Each team member can clone the repository and generate the dataset on their own machine.

---

# ⚙️ Setup

## 1. Clone the Repository

```bash
git clone https://github.com/Sahil1221ss/SearchEngine_using_DSA_concept-.git
```

Move into the project directory:

```bash
cd SearchEngine_using_DSA_concept-
```

---

## 2. Create a Python Virtual Environment

```bash
python -m venv .venv
```

---

## 3. Activate the Virtual Environment

### Windows PowerShell

```powershell
.venv\Scripts\activate
```

After activation, the terminal should look similar to:

```text
(.venv) PS C:\...\SearchEngine_using_DSA_concept->
```

---

## 4. Install Required Packages

```bash
pip install -r requirements.txt
```

---

# 📥 Data Collection

The project currently uses OpenAlex to collect academic-paper data.

Run:

```powershell
python src\data_collection\openalex_api.py
```

This collects the raw academic data.

The raw data is stored locally in:

```text
data/raw/
```

---

# 📄 Data Extraction

After collecting the raw data, extract the required paper information:

```powershell
python src\data_collection\data_extractor.py
```

This creates the processed paper dataset:

```text
data/processed/papers.csv
```

The current dataset contains approximately **5,000 academic papers**.

---

# 🔍 Data Quality Check

Before cleaning the dataset, run the quality-check program:

```powershell
python src\preprocessing\data_quality.py
```

This checks information such as:

* Total number of papers
* Missing titles
* Missing abstracts
* Missing authors
* Missing publication years
* Duplicate titles
* Publication-year range
* Abstract availability

---

# 🧹 Data Cleaning

Run:

```powershell
python src\preprocessing\clean_dataset.py
```

This processes the collected dataset and creates:

```text
data/processed/cleaned_papers.csv
```

---

# 🗂️ Building the Inverted Index

The current project uses an **Inverted Index** to organize words and the documents in which they occur.

Conceptually:

```text
machine  → [1, 4, 8, 12]
learning → [1, 3, 8]
network  → [2, 5, 9]
```

This allows the future search system to locate documents containing a particular word without scanning every paper individually.

Run:

```powershell
python src\indexing\inverted_index.py
```

The generated index is stored locally in:

```text
data/index/inverted_index.json
```

---

# 📁 Project Structure

```text
SearchEngine_using_DSA_concept-/
│
├── data/
│   ├── raw/
│   │   └── .gitkeep
│   ├── processed/
│   │   └── .gitkeep
│   └── index/
│       └── .gitkeep
│
├── docs/
│   ├── project_proposal.pdf
│   ├── system_design.md
│   └── dsa_mapping.md
│
├── src/
│   ├── data_collection/
│   │   ├── openalex_api.py
│   │   └── data_extractor.py
│   │
│   ├── preprocessing/
│   │   ├── data_quality.py
│   │   ├── clean_dataset.py
│   │   ├── text_cleaner.py
│   │   ├── tokenizer.py
│   │   └── stopwords.py
│   │
│   ├── indexing/
│   │   ├── inverted_index.py
│   │   └── index_builder.cpp
│   │
│   ├── search/
│   │   ├── search_engine.py
│   │   └── query_processor.cpp
│   │
│   ├── ranking/
│   │   └── result_ranker.cpp
│   │
│   ├── graph/
│   │   ├── bfs.cpp
│   │   ├── dfs.cpp
│   │   └── citation_graph.cpp
│   │
│   └── dsa/
│       ├── binary_search.cpp
│       ├── graph.cpp
│       ├── hash_table.cpp
│       ├── linked_list.cpp
│       ├── priority_queue.cpp
│       ├── sorting.cpp
│       └── trie.cpp
│
├── tests/
│   ├── test_graph.cpp
│   ├── test_index.cpp
│   ├── test_search.cpp
│   └── test_trie.cpp
│
├── .gitignore
├── README.md
└── requirements.txt
```

---

# 🚫 Dataset and Generated Files

The following files are intentionally **not uploaded to GitHub**:

```text
.venv/
data/raw/*
data/processed/*
data/index/*
```

These files are generated locally by running the project's scripts.

Therefore, after cloning the repository, team members need to run the data-collection and processing steps themselves.

---

# 🧪 Current Pipeline

The current working pipeline is:

```text
1. Collect academic data
        ↓
2. Extract paper information
        ↓
3. Check dataset quality
        ↓
4. Clean dataset
        ↓
5. Build inverted index
```

The current project has successfully generated an academic dataset and an inverted index containing thousands of unique terms.

---

# 🚧 Future Development

The project is currently being expanded into a complete academic search engine.

Planned components include:

```text
Inverted Index
      ↓
Query Processing
      ↓
Search
      ↓
Ranking
      ↓
Priority Queue
      ↓
Trie / Autocomplete
      ↓
Hash Table
      ↓
Citation Graph
      ↓
BFS / DFS
      ↓
Final Search Engine
```

As new features are implemented, this README will be updated with the corresponding setup and execution instructions.

---

## 👥 Team Development

The GitHub repository contains the source code and project documentation.

Team members should:

1. Clone the repository.
2. Create their own Python virtual environment.
3. Install the required packages.
4. Generate the academic dataset locally.
5. Run the preprocessing pipeline.
6. Work on their assigned DSA component.
7. Commit their changes.
8. Push their branch to GitHub.
9. Create a Pull Request for integration.

---

## 📌 Note

This project is under active development. The current README describes the **current implemented pipeline** and will be updated as additional search, ranking, and DSA functionality is integrated.
