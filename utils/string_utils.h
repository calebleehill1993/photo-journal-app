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

    bool stringContainsElement(std::string str, const char* elements) {
        for (int i = 0; i < strlen(elements); i++) {
            if (str.find(elements[i]) != std::string::npos) {
                return true;
            }
        }
        return false;
    }

} // namespace StringUtils

#endif // STRING_UTILS_H