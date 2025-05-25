#ifndef GOOGLE_DOCS_H
#define GOOGLE_DOCS_H

#include <string>

namespace GoogleDocsAPI {

    std::string getDocFile(const std::string& docId, const std::string& accessToken);

}

#endif // GOOGLE_DOCS_H