#ifndef GOOGLE_SHEETS_H
#define GOOGLE_SHEETS_H

#include <string>
#include <vector>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include "../utils/web_utils.h"

namespace GoogleSheetsAPI {

    void appendRowsToSheet(const std::string& accessToken,
                        const std::string& spreadsheetId,
                        const std::vector<std::vector<std::string>>& rowData) {
        CURL* curl = curl_easy_init();
        if (!curl) {
            std::cerr << "Failed to initialize CURL.\n";
            return;
        }

        std::string response_string;

        // Build URL
        std::string url = "https://sheets.googleapis.com/v4/spreadsheets/" + spreadsheetId +
                        "/values/Sheet1!A1:append?valueInputOption=RAW&insertDataOption=INSERT_ROWS";

        // JSON payload
        nlohmann::json j;
        j["values"] = rowData;

        std::string postData = j.dump();

        std::cout << postData << std::endl;

        // Headers
        std::string bearer = "Authorization: Bearer " + accessToken;
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, bearer.c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        // Set options
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WebUtils::WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            std::cerr << "Failed to append row: " << curl_easy_strerror(res) << "\n";
            return;
        }

        std::cout << "Response: " << response_string << "\n";
        return;
    }


    void sortSheetByDateTime(const std::string& accessToken, const std::string& spreadsheetId, int sheetId = 0) {
        CURL* curl = curl_easy_init();
        if (!curl) {
            std::cerr << "Failed to initialize CURL.\n";
            return;
        }

        std::string url = "https://sheets.googleapis.com/v4/spreadsheets/" + spreadsheetId + ":batchUpdate";
        std::string response_string;

        // Build sort request
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
                            { "dimensionIndex", 9 },  // Column J = UTC
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

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WebUtils::WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            std::cerr << "Sort request failed: " << curl_easy_strerror(res) << "\n";
            return;
        }

        std::cout << "Sort response: " << response_string << "\n";
        return;
    }

} // namespace google_sheets

#endif // GOOGLE_SHEETS_H