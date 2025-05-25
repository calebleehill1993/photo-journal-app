#ifndef CONFIG_CONSTANTS_H
#define CONFIG_CONSTANTS_H

#include <string>

namespace ConfigConstants {

    const std::string CONFIG_FILE_PATH = "config/config.json";

    const std::string GOOGLE_AUTH = "google_auth";
    const std::string CLIENT_ID = "client_id";
    const std::string CLIENT_SECRET = "client_secret";
    const std::string REDIRECT_URI = "redirect_uri";
    const std::string REFRESH_TOKEN = "refresh_token";

    const std::string SETTINGS = "settings";
    const std::string FONT_PATH = "font_path";
    const std::string ASPECT_RATIO_WIDTH = "aspect_ratio_width";
    const std::string ASPECT_RATIO_HEIGHT = "aspect_ratio_height";
    const std::string MARGINS = "margins";
    const std::string FONT_SIZE = "font_size";
    const std::string FONT_COLOR = "font_color";
    const std::string RED = "red";
    const std::string GREEN = "green";
    const std::string BLUE = "blue";
    const std::string TEXT_ALIGNMENT = "text_alignment";
    const std::string ALWAYS_USE_DESCENDER_SPACING = "always_use_descender_spacing";
    const std::string ADJUST_FOR_DAYLIGHT_SAVINGS = "adjust_for_daylight_savings";
    const std::string DEFAULT_TIME = "default_time";
    const std::string DEFAULT_TIMEZONE_OFFSET = "default_timezone_offset";
    const std::string GOOGLE_DOC_ID = "google_doc_id";
    const std::string GOOGLE_SHEET_ID = "google_sheet_id";
    const std::string PHOTOS_DESCRIPTION_CHAR_LIMIT = "photos_description_char_limit";

    const std::string DEBUG_SETTINGS = "debug_settings";
    const std::string SHOW_LINE_BORDERS = "show_line_borders";

} 

#endif // CONFIG_CONSTANTS_H