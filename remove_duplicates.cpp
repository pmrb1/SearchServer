#include "remove_duplicates.h"

void RemoveDuplicates(SearchServer& search_server) {
    std::map<std::set<std::string>, int> documents_container;
    std::vector<int> ids_to_remove;

    for (int document_id : search_server) {
        std::set<std::string> words;

        for (const auto& [word, _] : search_server.GetWordFrequencies(document_id)) {
            words.insert(std::string(word.begin(), word.end()));
        }

        if (!documents_container.emplace(words, document_id).second) {
            ids_to_remove.push_back(document_id);
        }
    }

    for (int id : ids_to_remove) {
        std::cout << "Found duplicate document id " << id << std::endl;
        search_server.RemoveDocument(id);
    }
}
