#include "time_utils.h"
#include <iostream>
#include <sstream>
#include <iomanip>
    
TimeUtils::TimeInfo TimeUtils::extractTimeInfo(const std::string& input) {
    std::smatch match;
    std::string time;
    if (regex_search(input, match, TIME_PATTERN)) {
        if (match[OPTIONAL_SECONDS].str().empty()) {
            time = match[TIME].str() + ":00";
        }
        
        else {
            time = match[TIME].str();
        }

        std::string offset = match[OPTIONAL_TIME_OFFSET].str();

        return {time, offset};
    }

    return {"", ""};
}

tm TimeUtils::getDstStart(const int year) {
    tm timeInfo = {};
    timeInfo.tm_year = year - 1900;
    timeInfo.tm_mon = 2;  // March (0-based index)
    timeInfo.tm_mday = 8; // Start search from March 8 (earliest possible second Sunday)
    timeInfo.tm_hour = 2; // 2 AM
    timeInfo.tm_min = 0;
    timeInfo.tm_sec = 0;

    mktime(&timeInfo); // Normalize to find the correct weekday
    while (timeInfo.tm_wday != 0) { // Find the first Sunday
        timeInfo.tm_mday++;
        mktime(&timeInfo);
    }

    return timeInfo;
}

tm TimeUtils::getDstEnd(const int year) {
    tm timeInfo = {};
    timeInfo.tm_year = year - 1900;
    timeInfo.tm_mon = 10; // November (0-based index)
    timeInfo.tm_mday = 1; // Start search from November 1
    timeInfo.tm_hour = 2; // 2 AM
    timeInfo.tm_min = 0;
    timeInfo.tm_sec = 0;

    mktime(&timeInfo); // Normalize to find the correct weekday
    while (timeInfo.tm_wday != 0) { // Find the first Sunday
        timeInfo.tm_mday++;
        mktime(&timeInfo);
    }

    return timeInfo;
}

tm TimeUtils::parseDatetime(const std::string& datetimeStr) {
    tm timeInfo = {};
    std::istringstream ss(datetimeStr);
    ss >> std::get_time(&timeInfo, "%Y-%m-%d %H:%M:%S");
    
    if (ss.fail()) {
        std::cerr << "Error: Invalid datetime format. Use YYYY-MM-DD HH:MM:SS" << std::endl;
    }
    
    return timeInfo;
}

bool TimeUtils::isDaylightSavingsTime(const std::string& datetimeStr) {
    tm inputTime = parseDatetime(datetimeStr);
    int year = inputTime.tm_year + 1900;

    // Get DST start and end dates
    tm dstStart = getDstStart(year);
    tm dstEnd = getDstEnd(year);

    // Convert to time_t for easy comparison
    time_t inputEpoch = mktime(&inputTime);
    time_t startEpoch = mktime(&dstStart);
    time_t endEpoch = mktime(&dstEnd);

    return inputEpoch >= startEpoch && inputEpoch < endEpoch;
}

std::string TimeUtils::adjustOffsetForDaylightSavings(const std::string& date, const std::string& time, const std::string& offset) {
    std::string datetime = date + " " + time;
    
    if (isDaylightSavingsTime(datetime)) {
        std::string sign = offset.substr(0, 1);
        std::string hour = offset.substr(1, 2);
        std::string minutes = offset.substr(4, 2);
        
        int intHour = stoi(sign + hour) + 1;
        
        if (intHour < 0) {
            sign = "-";
            intHour *= -1;
        } else {
            sign = "+";
        }
        
        hour = std::to_string(intHour);
        
        if (hour.length() < 2) {
            hour = "0" + hour;
        }
        
        return sign + hour + ":" + minutes;
    }
    
    return offset;
}

std::string TimeUtils::computeUtcDateTime(const std::string& date, const std::string& time, const std::string& timeOffset) {

    std::time_t now = std::time(nullptr);
    std::tm* localTm = std::localtime(&now);
    std::tm* utcTm = std::gmtime(&now);
    
    // Convert both to time_t again for difference (local time needs to be re-encoded)
    std::time_t localTt = std::mktime(localTm);  // Interprets tm as local time
    std::time_t utcTt = std::mktime(utcTm);      // Interprets tm as local time (so we treat utcTm as if it's local)

    int localTimeAdjustment = static_cast<int>(std::difftime(localTt, utcTt));
        
    // Parse "2025-05-09" and "22:34"
    std::tm tm = {};
    std::istringstream ss(date + " " + time);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M");

    // Convert to time_t in local time
    std::time_t local = std::mktime(&tm);

    // Parse offset (Ex: "-08:00" or "+05:30")
    int sign = (timeOffset[0] == '-') ? -1 : 1;
    int hours = std::stoi(timeOffset.substr(1, 2));
    int minutes = std::stoi(timeOffset.substr(4, 2));
    int offsetSeconds = sign * (hours * 3600 + minutes * 60);

    // Convert to UTC
    std::time_t utc = local - offsetSeconds + localTimeAdjustment;

    std::tm* gmt = std::gmtime(&utc);
    char buffer[25];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", gmt);

    return std::string(buffer);
}

std::string TimeUtils::getCurrentDatetime() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    if (ltm == nullptr) {
        std::cerr << "Error: Unable to get local time." << std::endl;
        return "";
    }

    // Format the date and time as "YYYY-MM-DD HH:MM:SS"
    std::stringstream datetime;
    datetime << 1900 + ltm->tm_year << "-"
                << std::setw(2) << std::setfill('0') << 1 + ltm->tm_mon << "-"
                << std::setw(2) << std::setfill('0') << ltm->tm_mday << " "
                << std::setw(2) << std::setfill('0') << ltm->tm_hour << ":"
                << std::setw(2) << std::setfill('0') << ltm->tm_min << ":"
                << std::setw(2) << std::setfill('0') << ltm->tm_sec;
    
    return datetime.str();
};
