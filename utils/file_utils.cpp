#include "file_utils.h"
#include <iostream>
#include <fstream>
#include <exiv2/exiv2.hpp>
#include <filesystem>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    #include <windows.h>
#else
    #include <sys/stat.h>
    #include <utime.h>
    #include <limits.h>
    #ifdef __APPLE__
        #include <mach-o/dyld.h>
    #elif __LINIX__
        #include <unistd.h>
    #endif
#endif

void FileUtils::deleteFile(const std::string& filename) {
    if (std::remove(filename.c_str()) != 0) {
        std::cerr << "Error deleting file: " << filename << std::endl;
    }
}

void FileUtils::setCurrentPath(const std::string& path) {
    #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        SetCurrentDirectory(path.c_str());
    #else
        std::filesystem::current_path(path);
    #endif
}

bool FileUtils::fileExists(const std::string& filename) {
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}

std::optional<std::string> FileUtils::readFile(const std::string& imagePath, const std::string& filename) {
    const std::string fullPath = imagePath + filename;
    if (!fileExists(fullPath)) {
        std::cerr << "File not found: " << fullPath << std::endl;
        return std::nullopt;
    }
    
    std::ifstream file(fullPath, std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file: " << fullPath << std::endl;
        return std::nullopt;
    }
    
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

std::string FileUtils::getExecutableDirectory() {
    #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        char path[MAX_PATH];
        GetModuleFileName(NULL, path, MAX_PATH);
        return std::filesystem::path(path).parent_path().string() + "/";
    #elif __APPLE__
        char path[PATH_MAX];
        uint32_t size = sizeof(path);
        if (_NSGetExecutablePath(path, &size) == 0) {
            return std::filesystem::path(path).parent_path().string() + "/";
        }
        return "";
    #elif __linux__
        char path[PATH_MAX];
        ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
        if (len != -1) {
            path[len] = '\0';
            return std::filesystem::path(path).parent_path().string() + "/";
        }
        return "";
    #else
    #error "Unknown compiler"
    #endif
}

void FileUtils::updateExifOriginalDate(const std::string& filename, const std::string& timestamp, const std::string& offset) {
    try {
        Exiv2::Image::UniquePtr image = Exiv2::ImageFactory::open(filename);
        if (!image) {
            std::cerr << "Error: Unable to open image file." << std::endl;
            return;
        }
        image->readMetadata();

        Exiv2::ExifData& exifData = image->exifData();

        exifData["Exif.Photo.DateTimeOriginal"] = timestamp;
        exifData["Exif.Photo.OffsetTimeOriginal"] = offset;

        image->writeMetadata();

        return;
    } catch (const Exiv2::Error& e) {
        std::cerr << "EXIF Update Error: " << e.what() << std::endl;
        return;
    }
}