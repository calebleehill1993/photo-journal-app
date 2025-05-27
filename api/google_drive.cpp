#include "google_drive.h"
#include <vector>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include "../utils/web_utils.h"

std::string GoogleDriveAPI::getDriveFile(const std::string& fileId, const std::string& accessToken) {
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

    curl_easy_setopt(curl, CURLOPT_URL, ("https://www.googleapis.com/drive/v3/files/" + fileId).c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WebUtils::writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::cerr << "Get File Failed: " << curl_easy_strerror(res) << std::endl;
        return "";
    }

    return response;
}

std::string GoogleDriveAPI::createGoogleSheetInFolder(const std::string& accessToken, const std::string& sheetName, const std::string& folderId) {
    CURL* curl = curl_easy_init();
    std::string response_string;
    CURLcode res;

    if (curl) {
        const std::string URL = "https://www.googleapis.com/drive/v3/files";
        const std::string JSON_NAME = "name";
        const std::string JSON_MIME_TYPE = "mimeType";
        const std::string JSON_PARENTS = "parents";
        const std::string MIME_TYPE = "application/vnd.google-apps.spreadsheet";

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        nlohmann::json j;
        j[JSON_NAME] = sheetName;
        j[JSON_MIME_TYPE] = MIME_TYPE;
        j[JSON_PARENTS] = nlohmann::json::array({ folderId });

        std::string postData = j.dump();

        curl_easy_setopt(curl, CURLOPT_URL, URL.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WebUtils::writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << "\n";
            return "";
        }
    }

    nlohmann::json result = nlohmann::json::parse(response_string);
    return result.value("id", "");
}