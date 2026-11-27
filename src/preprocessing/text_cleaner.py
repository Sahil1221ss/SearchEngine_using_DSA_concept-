
import re


def clean_text(text):
    """Clean text by converting it to lowercase and removing unwanted characters."""

    if not isinstance(text, str):
        return ""

    # Convert text to lowercase
    text = text.lower()

    # Remove URLs
    text = re.sub(r"https?://\S+|www\.\S+", "", text)

    # Keep only letters, numbers and spaces
    text = re.sub(r"[^a-z0-9\s]", " ", text)

    # Remove extra spaces
    text = re.sub(r"\s+", " ", text).strip()

    return text
