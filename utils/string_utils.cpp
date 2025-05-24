#include "string_utils.h"
#include <sstream>
#include <regex>

std::string StringUtils::trimSpaces(const std::string& str) {
    return std::regex_replace(str, std::regex("^\\s+|\\s+$"), "");
}

std::vector<std::string> StringUtils::splitSentence(std::string& sen) {
    std::stringstream ss(sen);
    std::string word;
    std::vector<std::string> words;

    while (ss >> word) {
        words.push_back(word);
    }
    
    return words;
}

std::vector<std::string> StringUtils::split(const std::string& str, const char delimiter) {
    std::vector<std::string> result;
    std::string token;
    std::istringstream tokenStream(str);

    while (getline(tokenStream, token, delimiter)) {
        result.push_back(token);
    }

    return result;
}

bool StringUtils::stringContainsElement(const std::string& str, const char* elements) {
    for (int i = 0; i < strlen(elements); i++) {
        if (str.find(elements[i]) != std::string::npos) {
            return true;
        }
    }
    return false;
}

bool StringUtils::isAllUppercaseLetters(const std::string& text) {
    for (const auto& c : text) {
        if (isalpha(c) && !isupper(c)) {
            return false;
        }
    }
    return true;
}
