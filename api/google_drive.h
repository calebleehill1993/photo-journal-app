#ifndef GOOGLE_DRIVE_H
#define GOOGLE_DRIVE_H

#include <string>

namespace GoogleDriveAPI {

    std::string getDriveFile(const std::string& fileId, const std::string& accessToken);
    std::string createGoogleSheetInFolder(const std::string& accessToken, const std::string& sheetName, const std::string& folderId);

} // namespace google_sheets

#endif // GOOGLE_DRIVE_H