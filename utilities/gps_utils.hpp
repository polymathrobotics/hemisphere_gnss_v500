#pragma once

#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>

namespace gps_utils
{

inline std::vector<std::string> split_fields_remove_checksum(const std::string& sentence)
{
    std::string payload = sentence;

    // Remove leading '$'
    if (!payload.empty() && payload.front() == '$') {
        payload.erase(payload.begin());
    }

    // Strip checksum
    const std::size_t star_pos = payload.find('*');
    if (star_pos != std::string::npos) {
        payload = payload.substr(0, star_pos);
    }

    std::vector<std::string> fields;
    std::stringstream ss(payload);
    std::string item;

    // Split by comma
    while (std::getline(ss, item, ',')) {
        fields.push_back(item);
    }

    if (!payload.empty() && payload.back() == ',') {
        fields.push_back("");
    }

    return fields;
}

inline double safe_to_double(const std::string& s, double fallback = 0.0)
{
    if (s.empty()) {
        return fallback;
    }

    char* end_ptr = nullptr;
    const double val = std::strtod(s.c_str(), &end_ptr);
    // If no conversion could be performed, return fallback
    return (end_ptr == s.c_str()) ? fallback : val;
}

inline int safe_to_int(const std::string& s, int fallback = 0)
{
    if (s.empty()) {
        return fallback;
    }

    char* end_ptr = nullptr;
    const long val = std::strtol(s.c_str(), &end_ptr, 10);
    return (end_ptr == s.c_str()) ? fallback : static_cast<int>(val);
}

inline double parse_nmea_latitude(const std::string& lat_str, const std::string& ns)
{
    if (lat_str.empty() || ns.empty()) return 0.0;

    const double raw = safe_to_double(lat_str, 0.0);
    const int degrees = static_cast<int>(raw / 100.0);
    const double minutes = raw - (static_cast<double>(degrees) * 100.0);
    double decimal_deg = static_cast<double>(degrees) + (minutes / 60.0);

    if (ns == "S") decimal_deg *= -1.0;
    return decimal_deg;
}

inline double parse_nmea_longitude(const std::string& lon_str, const std::string& ew)
{
    if (lon_str.empty() || ew.empty()) return 0.0;

    const double raw = safe_to_double(lon_str, 0.0);
    const int degrees = static_cast<int>(raw / 100.0);
    const double minutes = raw - (static_cast<double>(degrees) * 100.0);
    double decimal_deg = static_cast<double>(degrees) + (minutes / 60.0);

    if (ew == "W") decimal_deg *= -1.0;
    return decimal_deg;
}

inline double parse_nmea_utc_time_to_seconds(const std::string& utc_str)
{
    if (utc_str.size() < 6) return 0.0;

    const double raw = safe_to_double(utc_str, 0.0);
    const int hh = static_cast<int>(raw / 10000.0);
    const int mm = static_cast<int>((raw - (hh * 10000.0)) / 100.0);
    const double ss = raw - (hh * 10000.0) - (mm * 100.0);

    return static_cast<double>(hh * 3600 + mm * 60) + ss;
}

} // namespace gps_utils
