#include "process_queries.h"

#include <algorithm>
#include <execution>
#include <list>

std::vector<std::vector<Document>> ProcessQueries(const SearchServer& search_server,
                                                  const std::vector<std::string>& queries) {
    std::vector<std::vector<Document>> result(queries.size());
    std::transform(std::execution::par, queries.begin(), queries.end(), result.begin(),
                   [&search_server](std::string query) { return search_server.FindTopDocuments(query); });

    return result;
}

std::vector<Document> ProcessQueriesJoined(const SearchServer& search_server, const std::vector<std::string>& queries) {
    std::vector<Document> result;

    auto process_result = ProcessQueries(search_server, queries);

    for (const auto& documents : process_result) {
        result.insert(result.end(), documents.begin(), documents.end());
    }

    return result;
}