#ifndef GOOGLE_DOCS_H
#define GOOGLE_DOCS_H

#include <string>
#include <curl/curl.h>
#include <stdexcept>
#include <iostream>
#include "../utils/web_utils.h"

namespace GoogleDocsAPI {

    std::string getDocFile(const std::string& docId, const std::string& accessToken) {
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

        curl_easy_setopt(curl, CURLOPT_URL, ("https://docs.googleapis.com/v1/documents/" + docId).c_str());
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

        return response; // File Response
    }

} // namespace GoogleDocsAPI

#endif // GOOGLE_DOCS_H