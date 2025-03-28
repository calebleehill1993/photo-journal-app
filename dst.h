//
//  dst.h
//  google_photos_api_test
//
//  Created by Caleb Hill on 3/25/25.
//

#include <iostream>
#include <sstream>
#include <ctime>
#include <iomanip>

using namespace std;

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
tm parse_datetime(const string& datetime_str) {
    tm timeinfo = {};
    istringstream ss(datetime_str);
    ss >> get_time(&timeinfo, "%Y-%m-%d %H:%M:%S");
    
    if (ss.fail()) {
        cerr << "Error: Invalid datetime format. Use YYYY-MM-DD HH:MM:SS" << endl;
    }
    
    return timeinfo;
}

/**
 * Determines if Daylight Saving Time (DST) is active for a given datetime.
 */
bool is_daylight_savings_time(const string& datetime_str) {
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

string adjust_for_daylight_savings(const string& date, const string& time, const string& offset) {
    string datetime = date + " " + time;
    
    if (is_daylight_savings_time(datetime)) {
        string sign = offset.substr(0, 1);
        string hour = offset.substr(1, 2);
        string minutes = offset.substr(4, 2);
        
        int intHour = stoi(sign + hour) + 1;
        
        if (intHour < 0) {
            sign = "-";
            intHour *= -1;
        }
        else {
            sign = "+";
        }
        
        hour = to_string(intHour);
        
        if (hour.length() < 2) {
            hour = "0" + hour;
        }
        
        return sign + hour + ":" + minutes;
    }
    
    return offset;
}
