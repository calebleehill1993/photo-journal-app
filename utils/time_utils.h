#ifndef TIME_UTILS_H
#define TIME_UTILS_H

#include <string>
#include <ctime>
#include <regex>

namespace TimeUtils {

    struct TimeInfo {
        std::string time;
        std::string timeOffset;
    };
    
    TimeInfo extractTimeInfo(const std::string& input);
    tm getDstStart(const int year);
    tm getDstEnd(const int year);
    tm parseDatetime(const std::string& datetimeStr);
    bool isDaylightSavingsTime(const std::string& datetimeStr);
    std::string adjustOffsetForDaylightSavings(const std::string& date, const std::string& time, const std::string& offset);
    std::string computeUtcDateTime(const std::string& date, const std::string& time, const std::string& timeOffset);
    std::string getCurrentDatetime();

    const std::regex DATE_PATTERN(R"(\d{4}-\d{2}-\d{2})");
    const std::regex TIME_PATTERN(R"((\d{2}:\d{2}(:\d{2})?)([+-]\d{2}:\d{2})?)");

    enum TimePatternGroups {
        FULL_TIME = 0,
        TIME = 1,
        OPTIONAL_SECONDS = 2,
        OPTIONAL_TIME_OFFSET = 3
    };

}

#endif // TIME_UTILS_H
