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

    std::string getDoc() {
        authenticate();
        return GoogleDocsAPI::getDocFile(docId, accessToken);
    }

    std::string uploadPhoto(const std::string& project_path, std::string& filename, const std::string& description) {
        authenticate();

        std::string uploadToken = GooglePhotosAPI::uploadImage(accessToken, project_path, filename);

        if (!uploadToken.empty()) {
            std::string photosId = GooglePhotosAPI::createMediaItem(accessToken, uploadToken, filename, description);
            if (photosId.size() > 0) {
                std::cout << "Successfully uploaded image: " << photosId << std::endl;
            }

            return photosId;
        }
        
        else {
            std::cerr << "Failed to upload image." << std::endl;
            return "";
        }
    }

    void appendRowsToSheet(const std::vector<std::vector<std::string>>& rowData) {
        authenticate();

        GoogleSheetsAPI::appendRowsToSheet(accessToken, sheetId, rowData);
        GoogleSheetsAPI::sortSheetByDateTime(accessToken, sheetId);
    }

private:
    std::string accessToken;
    std::string docId;
    std::string sheetId;

    void authenticate() {
        if (accessToken.empty()) {
            accessToken = GoogleAuth::getInstance().getAccessToken();
        }
    };
    
};

#endif // GOOGLE_API_HANDLER_H