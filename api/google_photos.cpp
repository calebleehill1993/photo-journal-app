#include "google_photos.h"
#include <curl/curl.h>
#include <stdexcept>
#include <iostream>
#include <nlohmann/json.hpp>
#include "../utils/web_utils.h"
#include "../utils/file_utils.h"

std::string GooglePhotosAPI::uploadImage(const std::string& accessToken, const std::string& imagePath, const std::string& filename) {
    CURL* curl;
    CURLcode res;
    std::string response;
    
    curl = curl_easy_init();
    if (!curl) {
        std::cerr << "CURL initialization failed!" << std::endl;
        return "";
    }

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/octet-stream");
    headers = curl_slist_append(headers, "X-Goog-Upload-Protocol: raw");

    std::optional<std::string> fileDataOpt = FileUtils::readFile(imagePath, filename);
    if (!fileDataOpt.has_value()) {
        curl_easy_cleanup(curl);
        return "";
    }
    std::string fileData = fileDataOpt.value();
    
    curl_easy_setopt(curl, CURLOPT_URL, "https://photoslibrary.googleapis.com/v1/uploads");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, fileData.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, fileData.size());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WebUtils::writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::cerr << "Image upload failed: " << curl_easy_strerror(res) << std::endl;
        return "";
    }

    return response;
}

std::string GooglePhotosAPI::createMediaItem(const std::string& accessToken, const std::string& uploadToken, const std::string& filename, const std::string& description) {
    CURL* curl;
    CURLcode res;
    std::string response;
    
    nlohmann::json requestBody = {
        {"newMediaItems", {
            {
                {"description", description},
                {"simpleMediaItem", {
                    {"uploadToken", uploadToken},
                    {"fileName", filename}
                }}
            }
        }}
    };

    curl = curl_easy_init();
    if (!curl) {
        std::cerr << "CURL initialization failed!" << std::endl;
        return "";
    }

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    std::string postData = requestBody.dump();

    curl_easy_setopt(curl, CURLOPT_URL, "https://photoslibrary.googleapis.com/v1/mediaItems:batchCreate");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WebUtils::writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::cerr << "Failed to create media item: " << curl_easy_strerror(res) << std::endl;
        return "";
    }

    auto jsonResponse = nlohmann::json::parse(response);
    std::string photos_id = jsonResponse["newMediaItemResults"][0]["mediaItem"]["id"].get<std::string>();
    return photos_id;
}