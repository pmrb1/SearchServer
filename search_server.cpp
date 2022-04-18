#include "search_server.h"

#include <cassert>
#include <cmath>
#include <utility>

#include "string_processing.h"

SearchServer::SearchServer(const std::string_view stop_words_text)
    : SearchServer(string_processing::SplitIntoWords(std::string{stop_words_text.begin(), stop_words_text.end()})) {}

SearchServer::SearchServer(const std::string& stop_words_text)
    : SearchServer(string_processing::SplitIntoWords(stop_words_text)) {}

void SearchServer::SetStopWords(const std::string& text) {
    for (const std::string& word : string_processing::SplitIntoWords(text)) {
        stop_words_.insert(word);
    }
}

void SearchServer::AddDocument(int document_id, const std::string_view document, DocumentStatus document_status,
                               const std::vector<int>& document_ratings) {
    if (!IsValidDocumentId(document_id)) {
        throw std::invalid_argument("Error adding document. Invalid document id!");
    }
    DocumentData document_data{ComputeAverageRating(document_ratings), document_status,
                               std::move(std::string{document.begin(), document.end()})};

    documents_ids_.insert(document_id);
    documents_.emplace(document_id, document_data);

    const std::vector<std::string_view> words = SplitIntoWordsNoStop(documents_.at(document_id).raw_data);
    const double inverse_word_count = 1.0 / static_cast<int>(words.size());

    for (const std::string_view word : words) {
        word_to_document_frequencies_[word][document_id] += inverse_word_count;
        words_in_document_frequencies_[document_id][word] += inverse_word_count;
    }
}

[[nodiscard]] const std::vector<Document> SearchServer::FindTopDocuments(const std::string_view raw_query,
                                                                         DocumentStatus document_status) const {
    return FindTopDocuments(
        raw_query, [=](int document_id, DocumentStatus status, int rating) { return status == document_status; });
}

[[nodiscard]] int SearchServer::GetDocumentCount() const { return static_cast<int>(documents_.size()); }

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(std::string_view raw_query,
                                                                                      int document_id) const {
    return MatchDocument(std::execution::seq, raw_query, document_id);
}

[[nodiscard]] const std::map<std::string_view, double> SearchServer::GetWordFrequencies(int document_id) const {
    static std::map<std::string_view, double> empty_response;

    if (words_in_document_frequencies_.count(document_id) == 0) {
        return empty_response;
    }
    std::map<std::string_view, double> response;

    for (const auto& item : words_in_document_frequencies_.at(document_id)) {
        response.insert(item);
    }

    return response;
}

void SearchServer::RemoveDocument(int document_id) { return RemoveDocument(std::execution::seq, document_id); }

std::set<int>::const_iterator SearchServer::begin() const { return documents_ids_.begin(); }

std::set<int>::const_iterator SearchServer::end() const { return documents_ids_.end(); }

[[nodiscard]] bool SearchServer::IsValidWord(const std::string_view word) {
    return none_of(word.begin(), word.end(), [](char symbol) { return symbol >= '\0' && symbol < ' '; });
}

[[nodiscard]] bool SearchServer::IsValidDocumentId(const int& document_id) const {
    return document_id >= 0 || documents_.count(document_id) > 0;
}

[[nodiscard]] int SearchServer::ComputeAverageRating(const std::vector<int>& ratings) {
    int rating_sum = 0;

    if (ratings.size() == 0) {
        return 0;
    }

    for (int rating : ratings) {
        rating_sum += rating;
    }

    return rating_sum / static_cast<int>(ratings.size());
}

[[nodiscard]] bool SearchServer::IsStopWord(const std::string_view word) const {
    return stop_words_.count({word.begin(), word.end()}) > 0;
}

[[nodiscard]] const std::vector<std::string_view> SearchServer::SplitIntoWordsNoStop(
    const std::string_view text) const {
    std::vector<std::string_view> words;

    for (const std::string_view word : string_processing::SplitIntoWordsView(text)) {
        if (!IsValidWord(word)) {
            throw std::invalid_argument("Word contains invalid symblos");
        }

        if (!IsStopWord(word)) {
            words.push_back(word);
        }
    }

    return words;
}

[[nodiscard]] double SearchServer::ComputeWordInverseDocumentFrequency(const std::string_view word) const {
    assert(word_to_document_frequencies_.at(word).size() != 0);

    return log(GetDocumentCount() * 1.0 / word_to_document_frequencies_.at(word).size());
}

[[nodiscard]] const SearchServer::QueryWord SearchServer::ParseQueryWord(std::string_view text) const {
    bool is_minus = false;

    if (text[0] == '-') {
        is_minus = true;
        text = text.substr(1);

        if (text.empty()) {
            throw std::invalid_argument("Search error. Invalid query!");
        }
    }

    if (text.size() > 0 && text[0] == '-') {
        throw std::invalid_argument("Search error. Invalid query!");
    }

    if (IsValidWord(text)) {
        return {text, is_minus, IsStopWord(text)};
    }

    throw std::invalid_argument("Search error. Invalid query!");
}

[[nodiscard]] const SearchServer::Query SearchServer::ParseQuery(std::string_view text) const {
    if (!text.empty()) {
        Query query;

        for (const auto& word : string_processing::SplitIntoWordsView(text)) {
            const QueryWord query_word = ParseQueryWord(word);

            if (!query_word.is_stop) {
                query_word.is_minus ? query.minus_words.insert(query_word.data)
                                    : query.plus_words.insert(query_word.data);
            }
        }

        return query;
    }

    return {};
}
