#pragma once

#include <algorithm>
#include <execution>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "concurrent_map.h"
#include "document.h"
#include "log_duration.h"

class SearchServer {
public:
    template <typename StringContainer>
    explicit SearchServer(const StringContainer& stop_words) : stop_words_(MakeUniqueNonEmptyStrings(stop_words)) {
        if (!std::all_of(stop_words_.begin(), stop_words_.end(), IsValidWord)) {
            throw std::invalid_argument("Some of stop words are invalid");
        }
    }

    explicit SearchServer(const std::string_view stop_words_text);

    explicit SearchServer(const std::string& stop_words_text);

public:
    void SetStopWords(const std::string& text);

    void AddDocument(int document_id, const std::string_view document, DocumentStatus document_status,
                     const std::vector<int>& document_ratings);

    [[nodiscard]] const std::vector<Document> FindTopDocuments(
        const std::string_view raw_query, DocumentStatus document_status = DocumentStatus::kActual) const;

    template <typename Filter>
    [[nodiscard]] const std::vector<Document> FindTopDocuments(const std::string_view raw_query, Filter filter) const {
        return FindTopDocuments(std::execution::seq, raw_query, filter);
    }

    template <typename Filter, typename ExecutionPolicy>
    [[nodiscard]] const std::vector<Document> FindTopDocuments(ExecutionPolicy&& policy,
                                                               const std::string_view raw_query, Filter filter) const {
        static const double kAccuracy = 1e-6;

        const Query query = ParseQuery(raw_query);
        std::vector<Document> matched_documents = FindAllDocuments(policy, query, filter);

        std::sort(policy, matched_documents.begin(), matched_documents.end(),
                  [](const Document& lhs, const Document& rhs) {
                      if (std::abs(lhs.relevance - rhs.relevance) < kAccuracy) {
                          return lhs.rating > rhs.rating;
                      } else {
                          return lhs.relevance > rhs.relevance;
                      }
                  });

        if (static_cast<int>(matched_documents.size()) > kMaxResultDocumentCount) {
            matched_documents.resize(static_cast<size_t>(kMaxResultDocumentCount));
        }
        return matched_documents;
    }

    [[nodiscard]] int GetDocumentCount() const;

    [[nodiscard]] std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(std::string_view raw_query,
                                                                                          int document_id) const;

    template <class ExecutionPolicy>
    [[nodiscard]] std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(ExecutionPolicy&& policy,
                                                                                          std::string_view raw_query,
                                                                                          int document_id) const {
        if (documents_ids_.count(document_id) == 0) {
            throw std::out_of_range("non-existing document_id");
        }
        const Query query = ParseQuery(raw_query);
        std::vector<std::string_view> matched_words;

        const auto word_checker = [this, document_id](std::string_view word) {
            const auto it = word_to_document_frequencies_.find(word);
            return it != word_to_document_frequencies_.end() && it->second.count(document_id);
        };

        std::for_each(policy, query.plus_words.begin(), query.plus_words.end(),
                      [this, document_id, &matched_words, &word_checker](std::string_view word) {
                          if (word_checker(word)) {
                              matched_words.push_back(word);
                          }
                      });

        if (std::any_of(policy, query.minus_words.begin(), query.minus_words.end(), word_checker)) {
            matched_words.clear();

            return {matched_words, documents_.at(document_id).status};
        }
        std::sort(policy, matched_words.begin(), matched_words.end());
        auto last_it = std::unique(policy, matched_words.begin(), matched_words.end());

        matched_words.erase(last_it, matched_words.end());

        return {matched_words, documents_.at(document_id).status};
    }

    [[nodiscard]] const std::map<std::string_view, double> GetWordFrequencies(int document_id) const;

    void RemoveDocument(int document_id);

    template <class ExecutionPolicy>
    void RemoveDocument(ExecutionPolicy&& policy, int document_id) {
        if (documents_ids_.count(document_id) == 0) {
            return;
        }
        const auto& words_data = words_in_document_frequencies_.at(document_id);

        std::vector<std::string_view> word_ptrs(words_data.size());

        std::transform(policy, words_data.begin(), words_data.end(), word_ptrs.begin(),
                       [](auto word) { return word.first; });

        std::for_each(policy, word_ptrs.begin(), word_ptrs.end(), [this, document_id](std::string_view word_ptr) {
            word_to_document_frequencies_.at(word_ptr).erase(document_id);
        });

        documents_ids_.erase(document_id);

        words_in_document_frequencies_.erase(document_id);
        documents_.erase(document_id);
    }

    std::set<int>::const_iterator begin() const;

    std::set<int>::const_iterator end() const;

private:
    struct DocumentData {
        int rating = 0;
        DocumentStatus status = DocumentStatus::kActual;
        std::string raw_data;
    };

    struct QueryWord {
        std::string_view data;
        bool is_minus = false;
        bool is_stop = false;
    };

    struct Query {
        std::set<std::string_view> plus_words;
        std::set<std::string_view> minus_words;
    };

private:
    static const int kMaxResultDocumentCount = 5;
    static const size_t kBucketsNumber = 50;

private:
    template <typename StringContainer>
    [[nodiscard]] std::set<std::string> MakeUniqueNonEmptyStrings(const StringContainer& strings) {
        std::set<std::string> non_empty_strings;

        for (const std::string& word : strings) {
            if (!word.empty()) {
                if (IsValidWord(word)) {
                    non_empty_strings.insert(word);
                } else {
                    throw std::invalid_argument("stop words contains denied symbols");
                }
            }
        }
        return non_empty_strings;
    }

    [[nodiscard]] static bool IsValidWord(const std::string_view word);

    [[nodiscard]] bool IsValidDocumentId(const int& document_id) const;

    [[nodiscard]] static int ComputeAverageRating(const std::vector<int>& ratings);

    [[nodiscard]] bool IsStopWord(const std::string_view word) const;

    [[nodiscard]] const std::vector<std::string_view> SplitIntoWordsNoStop(const std::string_view text) const;

    [[nodiscard]] double ComputeWordInverseDocumentFrequency(const std::string_view word) const;

    [[nodiscard]] const QueryWord ParseQueryWord(std::string_view text) const;

    [[nodiscard]] const Query ParseQuery(const std::string_view text) const;

    template <typename Filter>
    [[nodiscard]] const std::vector<Document> FindAllDocuments(const Query& query, Filter query_filter) const {
        return FindAllDocuments(std::execution::seq, query, query_filter);
    }

    template <typename DocumentPredicate, typename ExecutionPolicy>
    [[nodiscard]] std::vector<Document> FindAllDocuments(ExecutionPolicy&& policy, const Query& query,
                                                         DocumentPredicate document_predicate) const {
        ConcurrentMap<int, double> documents_to_relevance(kBucketsNumber);

        for_each(policy, query.plus_words.begin(), query.plus_words.end(),
                 [this, &documents_to_relevance, &document_predicate](auto word) {
                     if (word_to_document_frequencies_.count(word) > 0) {
                         const double inverse_document_frequency = ComputeWordInverseDocumentFrequency(word);

                         for (const auto& [document_id, term_frequency] : word_to_document_frequencies_.at(word)) {
                             const auto& document_data = documents_.at(document_id);

                             if (document_predicate(document_id, document_data.status, document_data.rating)) {
                                 documents_to_relevance[document_id].ref_to_value +=
                                     term_frequency * inverse_document_frequency;
                             }
                         }
                     }
                 });

        for_each(policy, query.minus_words.begin(), query.minus_words.end(),
                 [this, &documents_to_relevance](auto word) {
                     if (word_to_document_frequencies_.count(word) > 0) {
                         for (const auto& [document_id, _] : word_to_document_frequencies_.at(word)) {
                             documents_to_relevance.Erase(document_id);
                         }
                     }
                 });
        std::vector<Document> matched_documents;

        for (const auto& [document_id, relevance] : documents_to_relevance.BuildOrdinaryMap()) {
            matched_documents.push_back({document_id, relevance, documents_.at(document_id).rating});
        }

        return matched_documents;
    }

private:
    std::set<std::string> stop_words_;
    std::map<std::string_view, std::map<int, double>> word_to_document_frequencies_;
    std::map<int, std::map<std::string_view, double>> words_in_document_frequencies_;
    std::map<int, DocumentData> documents_;
    std::set<int> documents_ids_;
};
