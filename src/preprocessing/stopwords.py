
STOPWORDS = {
    "a", "an", "the", "and", "or", "but",
    "is", "am", "are", "was", "were", "be",
    "been", "being", "to", "of", "in", "on",
    "at", "by", "for", "with", "from", "as",
    "it", "its", "this", "that", "these", "those",
    "do", "does", "did", "doing",
    "has", "have", "had",
    "can", "could", "will", "would",
    "should", "may", "might",
    "not", "no", "so", "if", "then",
    "than", "too", "very", "also",
    "what", "which", "who", "when",
    "where", "why", "how"
}


def remove_stopwords(words):
    return [word for word in words if word not in STOPWORDS]
