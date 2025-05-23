#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <string>
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

namespace FileUtils {

    void deleteFile(const std::string& filename) {
        if (std::remove(filename.c_str()) != 0) {
            std::cerr << "Error deleting file: " << filename << std::endl;
        } else {
            std::cout << "File deleted successfully: " << filename << std::endl;
        }
    }

    void setCurrentPath(const std::string& path) {
        #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
            SetCurrentDirectory(path.c_str());
        #else
            std::filesystem::current_path(path);
        #endif
    }

    bool fileExists(const std::string& filename) {
        struct stat buffer;
        return (stat(filename.c_str(), &buffer) == 0);
    }

    // Helper function to read file content
    std::string readFile(const std::string& imagePath, std::string& filename) {
        
        std::string fullPath = imagePath + filename;
        
        if (!fileExists(fullPath)) {
            std::cerr << "File not found: " << fullPath << std::endl;
            exit(1);
        }
        
        std::cout << fullPath << std::endl;
        
        std::ifstream file(fullPath, std::ios::binary);
        
        if (!file) {
            std::cerr << "Error opening file: " << fullPath << std::endl;
            exit(1);
        }
        
        return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    }

    std::string getExecutableDirectory() {
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

    /**
     * Updates the EXIF "DateTimeOriginal" field in a JPEG image.
     *
     * @param filename The file whose EXIF metadata should be updated.
     * @param timestamp The new timestamp in "YYYY:MM:DD HH:MM:SS" format.
     * @param offset The new offset for the timestamp in "+/-HH:MM" format.
     * @return True if successful, False otherwise.
     */
    void update_exif_original_date(const std::string& filename, const std::string& timestamp, const std::string& offset) {
        try {
            // Open the image file
            Exiv2::Image::UniquePtr image = Exiv2::ImageFactory::open(filename);
            if (!image) {
                std::cerr << "Error: Unable to open image file." << std::endl;
                return;
            }
            image->readMetadata();

            // Access EXIF metadata
            Exiv2::ExifData& exifData = image->exifData();

            // Modify the "DateTimeOriginal" field
            exifData["Exif.Photo.DateTimeOriginal"] = timestamp;
            exifData["Exif.Photo.OffsetTimeOriginal"] = offset;

            // Save changes
            image->writeMetadata();
            std::cout << "Successfully updated EXIF DateTimeOriginal to: " << timestamp << std::endl;
            std::cout << "Successfully updated EXIF OffsetTimeOriginal to: " << offset << std::endl;

            return;
        } 
        
        catch (const Exiv2::Error& e) {  // Corrected exception handling
            std::cerr << "EXIF Update Error: " << e.what() << std::endl;
            return;
        }
    }

}


#endif // FILE_UTILS_H