#include "read_input_functions.h"

#include <iostream>

namespace search_server_input {

std::string ReadLine() {
    std::string s;
    getline(std::cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result;
    std::cin >> result;
    ReadLine();
    return result;
}
}  // namespace search_server_input