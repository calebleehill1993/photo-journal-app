#ifndef GOOGLE_API_HANDLER_H
#define GOOGLE_API_HANDLER_H

#include <string>
#include <vector>

class GoogleAPIHandler {
public:
    GoogleAPIHandler();
    std::string getDoc();
    std::string uploadPhoto(const std::string& projectPath, std::string& filename, const std::string& description);
    void appendRowsToSheet(const std::vector<std::vector<std::string>>& rowData);

private:
    void authenticate();

private:
    std::string accessToken;
    std::string docId;
    std::string sheetId;
    
};

#endif // GOOGLE_API_HANDLER_H