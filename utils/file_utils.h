#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <string>
#include <optional>

namespace FileUtils {

    void deleteFile(const std::string& filename);
    void setCurrentPath(const std::string& path);
    bool fileExists(const std::string& filename);
    std::optional<std::string> readFile(const std::string& imagePath, const std::string& filename);
    std::string getExecutableDirectory();
    void updateExifOriginalDate(const std::string& filename, const std::string& timestamp, const std::string& offset);

}

#endif // FILE_UTILS_H