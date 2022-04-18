#pragma once

#include <deque>
#include <string>
#include <vector>

#include "document.h"
#include "search_server.h"

class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server) : search_server_(search_server) {}

    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
        return AddResultRequest(search_server_.FindTopDocuments(raw_query, document_predicate));
    }

    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);

    std::vector<Document> AddFindRequest(const std::string& raw_query);

    int GetNoResultRequests() const;

private:
    struct QueryResult {
        int documents_count = 0;
        int64_t query_timestamp = 0;
    };

private:
    const static int kMinutesInDay = 1440;

private:
    void AddResultRequest(int results_total);
    void AddRequest(int results_total);
    void RemoveRequest();

private:
    const SearchServer& search_server_;
    std::deque<QueryResult> requests_;
    int64_t timestamp_ = 0;
    int no_result_count_ = 0;
};
