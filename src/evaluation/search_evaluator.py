
def precision_at_k(relevant_results, k=10):

    if k <= 0:
        return 0

    top_results = relevant_results[:k]

    if not top_results:
        return 0

    relevant_count = sum(top_results)

    return relevant_count / len(top_results)


def recall_at_k(relevant_results, total_relevant, k=10):

    if total_relevant <= 0 or k <= 0:
        return 0

    top_results = relevant_results[:k]

    relevant_count = sum(top_results)

    return relevant_count / total_relevant

def f1_score(precision, recall):

    if precision + recall == 0:
        return 0

    return (2 * precision * recall) / (precision + recall)

def evaluate_search_results():

    print("SEARCH ENGINE EVALUATION")
    print("-----------------------")

    query = input("Enter search query: ")

    print("\nCheck the top 10 results from your search engine.")
    print("Enter 1 for relevant and 0 for not relevant.")

    relevance = []

    for i in range(1, 11):

        value = int(input(f"Result {i}: "))

        while value not in [0, 1]:
            value = int(input("Enter only 0 or 1: "))

        relevance.append(value)

    total_relevant = int(input("\nEnter total relevant papers in the dataset: "))

    precision = precision_at_k(relevance, 10)
    recall = recall_at_k(relevance, total_relevant, 10)
    f1 = f1_score(precision, recall)

    print("\nEvaluation Results")
    print("-----------------------")
    print("Query:", query)
    print("Precision@10:", round(precision, 2))
    print("Recall@10:", round(recall, 4))
    print("F1-score:", round(f1, 4))

if __name__ == "__main__":
    evaluate_search_results()