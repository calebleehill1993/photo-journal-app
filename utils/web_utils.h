#ifndef WEB_UTILS_H
#define WEB_UTILS_H

#include <string>

namespace WebUtils {

    // Function to handle cURL response
    size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* output);

}

#endif // WEB_UTILS_H