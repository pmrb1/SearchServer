#include "test.h"

#include <cassert>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "log_duration.h"
#include "process_queries.h"
#include "remove_duplicates.h"
#include "search_server.h"

using namespace std::string_literals;

void AssertImpl(bool value, const std::string& expr_str, const std::string& file, const std::string& func,
                unsigned line, const std::string& hint) {
    if (!value) {
        std::cout << file << "("s << line << "): "s << func << ": "s;
        std::cout << "ASSERT("s << expr_str << ") failed."s;
        if (!hint.empty()) {
            std::cout << " Hint: "s << hint;
        }
        std::cout << std::endl;
        abort();
    }
}

void TestSearchServerConstructorsForDeniedSymbols() {
    std::vector<std::string> stop_words_with_denied_symbols;
    std::string stop_words_string_with_denied_symbols;

    for (int i = 0; i < 32; ++i) {
        const char denied_symbol = char(i);

        stop_words_with_denied_symbols.push_back("any"s + denied_symbol);
        stop_words_string_with_denied_symbols += "any"s + denied_symbol + " "s;
    }

    try {
        SearchServer search_server(stop_words_with_denied_symbols);

        ASSERT_HINT(true,
                    "No exception detected! Constructor with denied symbols should "
                    "throw exception!");
    } catch (const std::invalid_argument& error) {
        ASSERT(error.what());
    }

    try {
        SearchServer search_server(stop_words_string_with_denied_symbols);
        ASSERT_HINT(true,
                    "No exception detected! Constructor with denied symbols should "
                    "throw exception!");
    } catch (const std::invalid_argument& error) {
        ASSERT(error.what());
    }
}

void TestAddDocumentsWithInvalidIds() {
    try {
        SearchServer search_server("at"s);
        search_server.AddDocument(20, "fluffy cat with fluffy tail"s, DocumentStatus::kActual, {7, 2, 7});
        search_server.AddDocument(20, "cat in the city"s, DocumentStatus::kActual, {1, 2, 3});
        ASSERT_HINT(true,
                    "No exception detected! Adding document with existing id "
                    "should throw exception!");
    } catch (const std::invalid_argument& error) {
        ASSERT(error.what());
    }

    try {
        SearchServer search_server("at"s);
        search_server.AddDocument(-1, "cat in the city"s, DocumentStatus::kActual, {1, 2, 3});
        ASSERT_HINT(true,
                    "No exception detected! Adding document with negative id "
                    "should throw exception!");
    } catch (const std::invalid_argument& error) {
        ASSERT(error.what());
    }
}

void TestAddDocumentsWithDeniedSymbols() {
    try {
        SearchServer search_server("at"s);

        for (int i = 0; i < 32; ++i) {
            const char denied_symbol = char(i);
            try {
                search_server.AddDocument(i, "cat in the city"s + denied_symbol, DocumentStatus::kActual, {1, 2, 3});
                ASSERT_HINT(true,
                            "No exception detected! Adding document with denied symbols "
                            "should throw exception!");
            } catch (const std::invalid_argument& error) {
                ASSERT(error.what());
            }
        }
    } catch (const std::invalid_argument& error) {
        ASSERT(error.what());
    }
}

void TestSearchDocumentsWithDeniedSymbols() {
    try {
        SearchServer search_server("at"s);

        search_server.AddDocument(0, "cat in the city"s, DocumentStatus::kActual, {1, 2, 3});
        search_server.AddDocument(1, "fluffy cat with fluffy tail"s, DocumentStatus::kActual, {7, 2, 7});

        for (int i = 0; i < 32; ++i) {
            const char denied_symbol = char(i);
            try {
                auto result = search_server.FindTopDocuments("cat"s + denied_symbol);
                ASSERT_HINT(result.size() > 0,
                            "No exception detected! Query with denied symbols should "
                            "throw exception!");
            } catch (const std::invalid_argument& error) {
                ASSERT(error.what());
            }
        }
    } catch (const std::invalid_argument& error) {
        ASSERT(error.what());
    }
}

void TestSearchDocumentsWithInvalidMinusWords() {
    try {
        SearchServer search_server("at"s);

        search_server.AddDocument(1, "well-groomed dog expressive eyes"s, DocumentStatus::kActual, {5, -12, 2, 1});
        search_server.AddDocument(2, "well-groomed starling Eugene"s, DocumentStatus::kActual, {9});

        const auto found_docks = search_server.FindTopDocuments("well-groomed -dog"s);
        ASSERT_EQUAL_HINT(found_docks.size(), 1, "Documents with minus words should be excluded from result");

        const auto empty_docks_response = search_server.FindTopDocuments("well-groomed -dog -starling"s);
        ASSERT_HINT(empty_docks_response.empty(), "Documents with minus words should be excluded from result");

        try {
            auto result = search_server.FindTopDocuments("-"s);
            ASSERT_HINT(result.size() > 0,
                        "No exception detected! Query with only minus symbol should "
                        "throw exception!");
        } catch (const std::invalid_argument& error) {
            ASSERT(error.what());
        }

        try {
            auto result = search_server.FindTopDocuments("--"s);
            ASSERT_HINT(result.size() > 0,
                        "No exception detected! Query with only minus symbols should "
                        "throw exception!");
        } catch (const std::invalid_argument& error) {
            ASSERT(error.what());
        }

        try {
            auto result = search_server.FindTopDocuments("- groomed"s);
            ASSERT_HINT(result.size() > 0,
                        "No exception detected! Space between minus symbol and word "
                        "should throw exception");
        } catch (const std::invalid_argument& error) {
            ASSERT(error.what());
        }
    } catch (const std::invalid_argument& error) {
        ASSERT(error.what());
    }
}
void TestMatchingDocumentsWithDeniedSymbols() {
    try {
        SearchServer search_server("at"s);

        search_server.AddDocument(0, "cat in the city"s, DocumentStatus::kActual, {1, 2, 3});
        search_server.AddDocument(1, "fluffy cat with fluffy tail"s, DocumentStatus::kActual, {7, 2, 7});

        for (int i = 0; i < 32; ++i) {
            const char denied_symbol = char(i);
            try {
                const auto [words, documents_status] = search_server.MatchDocument("cat collar"s + denied_symbol, 1);

                ASSERT_HINT(words.size() > 0,
                            "No exception detected! Matching with denied symbols should "
                            "throw exception!");
            } catch (const std::invalid_argument& error) {
                ASSERT(error.what());
            }
        }
    } catch (const std::invalid_argument& error) {
        ASSERT(error.what());
    }
}

void TestRemoveDuplicates() {
    {
        SearchServer search_server("and with"s);

        search_server.AddDocument(1, "funny pet and nasty rat"s, DocumentStatus::kActual, {7, 2, 7});
        search_server.AddDocument(2, "funny pet with curly hair"s, DocumentStatus::kActual, {1, 2});
        search_server.AddDocument(3, "funny pet with curly hair"s, DocumentStatus::kActual, {1, 2});
        search_server.AddDocument(4, "funny pet and curly hair"s, DocumentStatus::kActual, {1, 2});
        search_server.AddDocument(5, "funny funny pet and nasty nasty rat"s, DocumentStatus::kActual, {1, 2});
        search_server.AddDocument(6, "funny pet and not very nasty rat"s, DocumentStatus::kActual, {1, 2});
        search_server.AddDocument(7, "very nasty rat and not very funny pet"s, DocumentStatus::kActual, {1, 2});
        search_server.AddDocument(8, "pet with rat and rat and rat"s, DocumentStatus::kActual, {1, 2});
        search_server.AddDocument(9, "nasty rat with curly hair"s, DocumentStatus::kActual, {1, 2});

        RemoveDuplicates(search_server);

        ASSERT_EQUAL(search_server.GetDocumentCount(), 5);
    }
    {
        SearchServer search_server("and with"s);

        search_server.AddDocument(1, "funny pet and nasty rat"s, DocumentStatus::kActual, {7, 2, 7});
        search_server.AddDocument(2, "funny pet with curly hair"s, DocumentStatus::kActual, {1, 2});

        RemoveDuplicates(search_server);

        ASSERT_EQUAL(search_server.GetDocumentCount(), 2);
    }

    {
        SearchServer search_server("and with"s);

        search_server.AddDocument(2, "funny pet and nasty rat"s, DocumentStatus::kActual, {7, 2, 7});
        search_server.AddDocument(1, "funny pet and nasty rat"s, DocumentStatus::kActual, {1, 2});

        RemoveDuplicates(search_server);

        ASSERT_EQUAL(search_server.GetDocumentCount(), 1);
    }

    {
        SearchServer search_server("and with"s);

        search_server.AddDocument(1, "funny pet and nasty rat"s, DocumentStatus::kActual, {7, 2, 7});

        RemoveDuplicates(search_server);

        ASSERT_EQUAL(search_server.GetDocumentCount(), 1);
    }
}

void TestMatchingDocumentsWithInvalidMinusWords() {
    try {
        SearchServer search_server("at"s);

        search_server.AddDocument(0, "cat in the city"s, DocumentStatus::kActual, {1, 2, 3});
        search_server.AddDocument(1, "white cat and fashionable collar"s, DocumentStatus::kActual, {8, -3});

        try {
            const auto [words, documents_status] = search_server.MatchDocument(std::execution::par, "-"s, 1);
            ASSERT_HINT(words.size(),
                        "No exception detected! Matching with only minus symbol should "
                        "throw exception!");
        } catch (const std::invalid_argument& error) {
            ASSERT(error.what());
        }

        try {
            const auto [words, documents_status] = search_server.MatchDocument(std::execution::seq, "--"s, 1);
            ASSERT_HINT(words.size(),
                        "No exception detected! Matching with only minus symbols "
                        "should throw exception!");
        } catch (const std::invalid_argument& error) {
            ASSERT(error.what());
        }

        try {
            const auto [words, documents_status] = search_server.MatchDocument("- groomed"s, 1);
            ASSERT_HINT(words.size(),
                        "No exception detected! Space between minus symbol and word "
                        "should throw exception");
        } catch (const std::invalid_argument& error) {
            ASSERT(error.what());
        }
    } catch (const std::invalid_argument& error) {
        ASSERT(error.what());
    }
}

void TestParallelQueries() {
    SearchServer search_server("and with"s);

    for (int id = 0; const std::string& text : {
                         "white cat and yellow hat"s,
                         "curly cat curly tail"s,
                         "nasty dog with big eyes"s,
                         "nasty pigeon john"s,
                     }) {
        search_server.AddDocument(++id, text, DocumentStatus::kActual, {1, 2});
    }

    std::vector<Document> document = search_server.FindTopDocuments("curly nasty cat"s);

    ASSERT_HINT(document.size() == 4, "FindTopDocuments has error");

    std::vector<Document> documents_banned =
        search_server.FindTopDocuments("curly nasty cat"s, DocumentStatus::kBanned);

    ASSERT_HINT(documents_banned.size() == 0, "FindTopDocuments with execution::seq has error");

    std::vector<Document> documents_even_ids = search_server.FindTopDocuments(
        std::execution::par, "curly nasty cat"s,
        [](int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0; });

    ASSERT_HINT(documents_even_ids.size() == 2, "FindTopDocuments with execution::par has error");
}

void TestSearchServer() {
    RUN_TEST(TestSearchServerConstructorsForDeniedSymbols);
    RUN_TEST(TestAddDocumentsWithInvalidIds);
    RUN_TEST(TestAddDocumentsWithDeniedSymbols);
    RUN_TEST(TestSearchDocumentsWithDeniedSymbols);
    RUN_TEST(TestSearchDocumentsWithInvalidMinusWords);
    RUN_TEST(TestMatchingDocumentsWithDeniedSymbols);
    RUN_TEST(TestMatchingDocumentsWithInvalidMinusWords);
    RUN_TEST(TestRemoveDuplicates);
    RUN_TEST(TestParallelQueries);
}
