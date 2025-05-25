#include "google_api_handler.h"
#include <iostream>
#include "google_auth.h"
#include "google_docs.h"
#include "google_sheets.h"
#include "google_photos.h"
#include "google_drive.h"
#include "../config/config_handler.h"

namespace ConfigConst = ConfigConstants;

GoogleAPIHandler::GoogleAPIHandler() {
    docId = ConfigHandler::getInstance().getConfigValue(ConfigConst::SETTINGS, ConfigConst::GOOGLE_DOC_ID);
    sheetId = ConfigHandler::getInstance().getConfigValue(ConfigConst::SETTINGS, ConfigConst::GOOGLE_SHEET_ID);
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
            std::cout << "Successfully uploaded image: " << filename << std::endl;
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

    std::cout << "Successfully appended " << rowData.size() << " rows to Google Sheet." << std::endl;
}

void GoogleAPIHandler::authenticate() {
    if (accessToken.empty()) {
        accessToken = GoogleAuth::getInstance().getAccessToken();
    }
}