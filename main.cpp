#include "log_duration.h"
#include "paginator.h"
#include "process_queries.h"
#include "request_queue.h"
#include "search_server.h"
#include "test.h"

using namespace std::string_literals;

int main() {
    TestSearchServer();
    SearchServer search_server("and with"s);

    int id = 0;
    for (const std::string& text : {
             "funny pet and nasty rat"s,
             "funny pet with curly hair"s,
             "funny pet and not very nasty rat"s,
             "pet with rat and rat and rat"s,
             "nasty rat with curly hair"s,
         }) {
        search_server.AddDocument(++id, text, DocumentStatus::kActual, {1, 2});
    }

    const std::vector<std::string> queries = {"nasty rat -not"s, "not very funny nasty pet"s, "curly hair"s};
    for (const Document& document : ProcessQueriesJoined(search_server, queries)) {
        std::cout << "Document "s << document.id << " matched with relevance "s << document.relevance << std::endl;
    }

    return 0;
}