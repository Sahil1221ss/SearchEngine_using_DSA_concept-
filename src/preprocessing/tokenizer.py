
import re
from nltk.stem import PorterStemmer

from src.preprocessing.stopwords import remove_stopwords
from src.preprocessing.text_cleaner import clean_text
import sys
from pathlib import Path

ROOT_DIR = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT_DIR))


# Create stemming tool
stemmer = PorterStemmer()


def tokenize(text):
    """Clean, split, remove stopwords and stem words."""

    # Step 1: Clean the text
    text = clean_text(text)

    # Step 2: Split text into individual words
    words = re.findall(r"\b[a-z0-9]+\b", text)

    # Step 3: Remove common stopwords
    words = remove_stopwords(words)

    # Step 4: Apply stemming
    words = [stemmer.stem(word) for word in words]

    return words
