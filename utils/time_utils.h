#ifndef TIME_UTILS_H
#define TIME_UTILS_H

#include <iostream>
#include <sstream>
#include <ctime>
#include <iomanip>

namespace TimeUtils {

    /**
     * Computes the date of the second Sunday of March for a given year.
     */
    tm get_dst_start(int year) {
        tm timeinfo = {};
        timeinfo.tm_year = year - 1900;
        timeinfo.tm_mon = 2;  // March (0-based index)
        timeinfo.tm_mday = 8; // Start search from March 8 (earliest possible second Sunday)
        timeinfo.tm_hour = 2; // 2 AM
        timeinfo.tm_min = 0;
        timeinfo.tm_sec = 0;

        mktime(&timeinfo); // Normalize to find the correct weekday
        while (timeinfo.tm_wday != 0) { // Find the first Sunday
            timeinfo.tm_mday++;
            mktime(&timeinfo);
        }

        return timeinfo;
    }

    /**
     * Computes the date of the first Sunday of November for a given year.
     */
    tm get_dst_end(int year) {
        tm timeinfo = {};
        timeinfo.tm_year = year - 1900;
        timeinfo.tm_mon = 10; // November (0-based index)
        timeinfo.tm_mday = 1; // Start search from November 1
        timeinfo.tm_hour = 2; // 2 AM
        timeinfo.tm_min = 0;
        timeinfo.tm_sec = 0;

        mktime(&timeinfo); // Normalize to find the correct weekday
        while (timeinfo.tm_wday != 0) { // Find the first Sunday
            timeinfo.tm_mday++;
            mktime(&timeinfo);
        }

        return timeinfo;
    }

    /**
     * Parses a datetime string in "YYYY-MM-DD HH:MM:SS" format into a `tm` struct.
     */
    tm parse_datetime(const std::string& datetime_str) {
        tm timeinfo = {};
        std::istringstream ss(datetime_str);
        ss >> std::get_time(&timeinfo, "%Y-%m-%d %H:%M:%S");
        
        if (ss.fail()) {
            std::cerr << "Error: Invalid datetime format. Use YYYY-MM-DD HH:MM:SS" << std::endl;
        }
        
        return timeinfo;
    }

    /**
     * Determines if Daylight Saving Time (DST) is active for a given datetime.
     */
    bool is_daylight_savings_time(const std::string& datetime_str) {
        tm input_time = parse_datetime(datetime_str);
        int year = input_time.tm_year + 1900;

        // Get DST start and end dates
        tm dst_start = get_dst_start(year);
        tm dst_end = get_dst_end(year);

        // Convert to time_t for easy comparison
        time_t input_epoch = mktime(&input_time);
        time_t start_epoch = mktime(&dst_start);
        time_t end_epoch = mktime(&dst_end);

        return input_epoch >= start_epoch && input_epoch < end_epoch;
    }

    std::string adjust_for_daylight_savings(const std::string& date, const std::string& time, const std::string& offset) {
        std::string datetime = date + " " + time;
        
        if (is_daylight_savings_time(datetime)) {
            std::string sign = offset.substr(0, 1);
            std::string hour = offset.substr(1, 2);
            std::string minutes = offset.substr(4, 2);
            
            int intHour = stoi(sign + hour) + 1;
            
            if (intHour < 0) {
                sign = "-";
                intHour *= -1;
            }
            
            else {
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

    std::string computeUtcDateTime(const std::string& date,
        const std::string& time,
        const std::string& timeOffset) {

        std::time_t now = std::time(nullptr);
        std::tm local_tm = *std::localtime(&now);
        std::tm utc_tm = *std::gmtime(&now);
        
        // Convert both to time_t again for difference (local time needs to be re-encoded)
        std::time_t local_tt = std::mktime(&local_tm);  // Interprets tm as local time
        std::time_t utc_tt = std::mktime(&utc_tm);      // Interprets tm as local time (so we treat utc_tm as if it's local)

        int localTimeAdjustment = static_cast<int>(std::difftime(local_tt, utc_tt));
            
        // Parse "2025-05-09" and "22:34"
        std::tm tm = {};
        std::istringstream ss(date + " " + time);
        ss >> std::get_time(&tm, "%Y-%m-%d %H:%M");

        // Convert to time_t in local time
        std::time_t local = std::mktime(&tm);

        // Parse offset "-08:00" or "+05:30"
        int sign = (timeOffset[0] == '-') ? -1 : 1;
        int hours = std::stoi(timeOffset.substr(1, 2));
        int minutes = std::stoi(timeOffset.substr(4, 2));
        int offsetSeconds = sign * (hours * 3600 + minutes * 60);

        // Convert to UTC
        std::time_t utc = local - offsetSeconds + localTimeAdjustment;

        std::tm* gmt = std::gmtime(&utc);
        char buffer[25];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", gmt);

        return std::string(buffer);  // ISO-like UTC timestamp
    }

} // namespace TimeUtils

#endif // TIME_UTILS_H