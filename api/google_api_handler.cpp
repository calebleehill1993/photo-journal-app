#include "google_api_handler.h"
#include <iostream>
#include "google_auth.h"
#include "google_docs.h"
#include "google_sheets.h"
#include "google_photos.h"
#include "google_drive.h"
#include "../config/config_handler.h"

GoogleAPIHandler::GoogleAPIHandler() {
    docId = ConfigHandler::getInstance().getConfigValue("settings", "google_doc_id");
    sheetId = ConfigHandler::getInstance().getConfigValue("settings", "google_sheet_id");
};

std::string GoogleAPIHandler::getDoc() {
    authenticate();
    return GoogleDocsAPI::getDocFile(docId, accessToken);
}

std::string GoogleAPIHandler::uploadPhoto(const std::string& projectPath, std::string& filename, const std::string& description) {
    authenticate();

    std::string uploadToken = GooglePhotosAPI::uploadImage(accessToken, projectPath, filename);

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

void GoogleAPIHandler::appendRowsToSheet(const std::vector<std::vector<std::string>>& rowData) {
    authenticate();

    GoogleSheetsAPI::appendRowsToSheet(accessToken, sheetId, rowData);
    GoogleSheetsAPI::sortSheetByDateTime(accessToken, sheetId);
}

void GoogleAPIHandler::authenticate() {
    if (accessToken.empty()) {
        accessToken = GoogleAuth::getInstance().getAccessToken();
    }
}