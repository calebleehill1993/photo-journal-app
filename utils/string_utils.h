#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <string>
#include <vector>
#include <sstream>

namespace StringUtils {

    std::vector<std::string> splitSentence(std::string& sen) {
        std::stringstream ss(sen);
        std::string word;
        std::vector<std::string> words;
    
        while (ss >> word) {
            words.push_back(word);
        }
        
        return words;
    }

    std::vector<std::string> split(const std::string& str, char delimiter) {
        std::vector<std::string> result;
        std::string token;
        std::istringstream tokenStream(str);
    
        while (getline(tokenStream, token, delimiter)) {
            result.push_back(token);
        }
    
        return result;
    }

    bool stringContainsElement(std::string str, const char* elements) {
        for (int i = 0; i < strlen(elements); i++) {
            if (str.find(elements[i]) != std::string::npos) {
                return true;
            }
        }
        return false;
    }

    bool isAllUppercaseLetters(const std::string& text) {
        for (char c : text) {
            if (isalpha(c) && !isupper(c)) {
                return false;
            }
        }
        return true;
    }

} // namespace StringUtils

#endif // STRING_UTILS_H