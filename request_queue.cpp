#include "request_queue.h"

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentStatus status) {
    auto search_result = search_server_.FindTopDocuments(raw_query, status);

    AddResultRequest(search_result.size());

    return search_result;
}

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query) {
    auto search_result = search_server_.FindTopDocuments(raw_query);

    AddResultRequest(search_result.size());

    return search_result;
}

int RequestQueue::GetNoResultRequests() const { return no_result_count_; }

void RequestQueue::AddResultRequest(int results_total) {
    timestamp_++;

    bool old_request_exist = !requests_.empty() && requests_.front().query_timestamp <=
                                                       requests_.back().query_timestamp - (kMinutesInDay - 1);

    if (old_request_exist) {
        RemoveRequest();
    }
    AddRequest(results_total);
}

void RequestQueue::RemoveRequest() {
    if (requests_.front().documents_count == 0) {
        --no_result_count_;
    }
    requests_.pop_front();
}

void RequestQueue::AddRequest(int results_total) {
    requests_.push_back({results_total, timestamp_});

    if (results_total == 0) {
        ++no_result_count_;
    }
}
