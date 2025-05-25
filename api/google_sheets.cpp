#include "google_sheets.h"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include "../utils/web_utils.h"

void GoogleSheetsAPI::appendRowsToSheet(const std::string& accessToken, const std::string& spreadsheetId, const std::vector<std::vector<std::string>>& rowData) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize CURL.\n";
        return;
    }

    std::string response_string;
    std::string url = "https://sheets.googleapis.com/v4/spreadsheets/" + spreadsheetId +
                    "/values/Sheet1!A1:append?valueInputOption=RAW&insertDataOption=INSERT_ROWS";

    nlohmann::json j;
    j["values"] = rowData;

    std::string postData = j.dump();

    std::string bearer = "Authorization: Bearer " + accessToken;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, bearer.c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WebUtils::writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::cerr << "Failed to append row: " << curl_easy_strerror(res) << "\n";
    }
}


void GoogleSheetsAPI::sortSheetByDateTime(const std::string& accessToken, const std::string& spreadsheetId, int sheetId) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize CURL.\n";
        return;
    }

    std::string url = "https://sheets.googleapis.com/v4/spreadsheets/" + spreadsheetId + ":batchUpdate";
    std::string response_string;

    const int UTC_DATETIME_COLUMN_INDEX = 9;
    nlohmann::json sortRequest = {
        { "requests", {
            {
            { "sortRange", {
                { "range", {
                    { "sheetId", sheetId },
                    { "startRowIndex", 1 }
                }},
                { "sortSpecs", {
                    {
                        { "dimensionIndex", UTC_DATETIME_COLUMN_INDEX },
                        { "sortOrder", "DESCENDING" }
                    }
                }}
            }}
            }
        }}
    };

    std::string postData = sortRequest.dump();

    std::string bearer = "Authorization: Bearer " + accessToken;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, bearer.c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WebUtils::writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::cerr << "Sort request failed: " << curl_easy_strerror(res) << "\n";
    }
}