#ifndef GOOGLE_SHEETS_H
#define GOOGLE_SHEETS_H

#include <string>
#include <vector>

namespace GoogleSheetsAPI {

    void appendRowsToSheet(const std::string& accessToken, const std::string& spreadsheetId, const std::vector<std::vector<std::string>>& rowData);
    void sortSheetByDateTime(const std::string& accessToken, const std::string& spreadsheetId, int sheetId = 0);

} // namespace google_sheets

#endif // GOOGLE_SHEETS_H