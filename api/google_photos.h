#ifndef GOOGLE_PHOTOS_H
#define GOOGLE_PHOTOS_H

#include <string>

namespace GooglePhotosAPI {

    std::string uploadImage(const std::string& accessToken, const std::string& imagePath, const std::string& filename);
    std::string createMediaItem(const std::string& accessToken, const std::string& uploadToken, const std::string& filename, const std::string& description);

}

#endif // GOOGLE_PHOTOS_H