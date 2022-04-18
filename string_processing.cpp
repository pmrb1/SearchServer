#include "string_processing.h"

#include <string_view>

namespace string_processing {

std::vector<std::string> SplitIntoWords(const std::string& text) {
    std::vector<std::string> words;
    std::string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }
    return words;
}

std::vector<std::string_view> SplitIntoWordsView(std::string_view text) {
    std::vector<std::string_view> result;
    int64_t pos = 0;
    const int64_t pos_end = text.npos;

    while (true) {
        int64_t space = text.find(' ', pos);
        result.push_back(space == pos_end ? text.substr(pos) : text.substr(pos, space - pos));

        if (space == pos_end) {
            break;
        } else {
            pos = space + 1;
        }
    }

    return result;
}

}  // namespace string_processing