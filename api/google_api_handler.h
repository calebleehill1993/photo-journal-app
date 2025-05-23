#ifndef GOOGLE_API_HANDLER_H
#define GOOGLE_API_HANDLER_H

#include <string>
#include <vector>
#include "google_auth.h"
#include "google_docs.h"
#include "google_sheets.h"
#include "google_photos.h"
#include "google_drive.h"
#include "../config/config_handler.h"

class GoogleAPIHandler {
public:
    GoogleAPIHandler() {
        docId = ConfigHandler::getInstance().getConfigValue("settings", "google_doc_id");
        sheetId = ConfigHandler::getInstance().getConfigValue("settings", "google_sheet_id");
    };

    ~GoogleAPIHandler() = default;

    void authenticate() {
        accessToken = GoogleAuth::getInstance().getAccessToken();
    };

    void checkAuthentication() {
        if (accessToken.empty()) {
            authenticate();
        }
    }

    std::string getDoc() {
        checkAuthentication();
        return GoogleDocsAPI::getDocFile(docId, accessToken);
    }

    std::string uploadPhoto(const std::string& project_path, std::string& filename, const std::string& description) {
        checkAuthentication();

        std::string uploadToken = GooglePhotosAPI::uploadImage(accessToken, project_path, filename);
        if (!uploadToken.empty()) {
            cout << "Upload Token: " << uploadToken << endl;
            std::string photosId = GooglePhotosAPI::createMediaItem(accessToken, uploadToken, filename, description);
            if (photosId.size() > 0) {
                cout << "Image " << photosId << " uploaded successfully!" << endl;
            }
            return photosId;
        } else {
            cerr << "Failed to upload image." << endl;
            return "";
        }
    }

    void appendRowsToSheet(const std::vector<std::vector<std::string>>& rowData) {
        checkAuthentication();
        GoogleSheetsAPI::appendRowsToSheet(accessToken, sheetId, rowData);
        GoogleSheetsAPI::sortSheetByDateTime(accessToken, sheetId);
    }



private:

    // Store authentication token
    std::string accessToken;
    std::string docId;
    std::string sheetId;
};

#endif // GOOGLE_API_HANDLER_H