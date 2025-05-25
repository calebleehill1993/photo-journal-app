#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <string>
#include <vector>

namespace StringUtils {

    std::string trimSpaces(const std::string& str);
    std::vector<std::string> splitSentence(std::string& sen);
    std::vector<std::string> split(const std::string& str, const char delimiter);
    bool stringContainsElement(const std::string& str, const std::string elements);
    bool isAllUppercaseLetters(const std::string& text);

}

#endif // STRING_UTILS_H