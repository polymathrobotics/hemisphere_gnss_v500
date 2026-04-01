#include "nmea_parser.hpp"

namespace hemisphere_gnss_v500_driver
{

NMEA_PARSER::NMEA_PARSER() {}

NMEA_PARSER::~NMEA_PARSER() {}


// TODO @aarush5 Handle case for heading and publish as a sensor_msgs/Imu message soon

NMEAParseResult NMEA_PARSER::on_nmea_parse(const std::string& nmea_sentence)
{
    // --- 1. Handle GST (Covariance) ---
    if (is_gst_sentence(nmea_sentence)) {
        last_gst_ = parse_gst(nmea_sentence);

        // If we already have a valid GGA from this same timestamp, bundle them now
        if (last_gst_.is_valid && last_gga_.is_valid &&
            std::abs(last_gst_.timestamp - last_gga_.timestamp) < 0.01) {

            NavSatFix combined;
            combined.position = last_gga_;
            combined.covariance = last_gst_;

            // Sync std_devs into position struct
            combined.position.lat_std_dev = last_gst_.lat_std_dev;
            combined.position.lon_std_dev = last_gst_.lon_std_dev;
            combined.position.alt_std_dev = last_gst_.alt_std_dev;

            return combined;
        }

        // Just updated cache, don't return anything yet
        return std::monostate{};
    }

    // --- 2. Handle GGA (Position) ---
    if (is_gga_sentence(nmea_sentence)) {
        last_gga_ = parse_gga(nmea_sentence); // Update class member

        // If we already have a valid GST from this same timestamp, bundle them
        if (last_gga_.is_valid && last_gst_.is_valid &&
            std::abs(last_gga_.timestamp - last_gst_.timestamp) < 0.01) {

            NavSatFix combined;
            combined.position = last_gga_;
            combined.covariance = last_gst_;

            combined.position.lat_std_dev = last_gst_.lat_std_dev;
            combined.position.lon_std_dev = last_gst_.lon_std_dev;
            combined.position.alt_std_dev = last_gst_.alt_std_dev;

            return combined;
        }

        // Return just the position if GST hasn't arrived yet or timestamp is off
        return last_gga_;
    }

    // --- 3. Handle Heading ---
    if (is_heading_sentence(nmea_sentence)) {
        return parse_heading(nmea_sentence);
    }

    return std::monostate{};
}

bool NMEA_PARSER::is_gga_sentence(const std::string& sentence) const
{
    return sentence.rfind("$GPGGA,", 0) == 0 || sentence.rfind("$GNGGA,", 0) == 0;
}

GPSPositionStruct NMEA_PARSER::parse_gga(const std::string& sentence) const
{
    GPSPositionStruct out{};
    out.is_valid = false;

    const std::vector<std::string> fields = gps_utils::split_fields_remove_checksum(sentence);

    if (fields.size() < 10) {
        return out;
    }

    out.timestamp = gps_utils::parse_nmea_utc_time_to_seconds(fields[1]);
    out.latitude = gps_utils::parse_nmea_latitude(fields[2], fields[3]);
    out.longitude = gps_utils::parse_nmea_longitude(fields[4], fields[5]);
    out.fix_quality = gps_utils::safe_to_int(fields[6], 0);
    out.num_satellites = gps_utils::safe_to_int(fields[7], 0);
    out.hdop = gps_utils::safe_to_double(fields[8], 99.9);
    out.altitude = gps_utils::safe_to_double(fields[9], 0.0);

    out.lat_std_dev = 0.0;
    out.lon_std_dev = 0.0;
    out.alt_std_dev = 0.0;

    out.is_valid = (out.fix_quality > 0);

    return out;
}

bool NMEA_PARSER::is_gst_sentence(const std::string& sentence) const
{
    return sentence.rfind("$GPGST,", 0) == 0 || sentence.rfind("$GNGST,", 0) == 0;
}

GPSCovarianceStruct NMEA_PARSER::parse_gst(const std::string& sentence) const
{
    GPSCovarianceStruct out{};
    out.is_valid = false;

    const std::vector<std::string> fields = gps_utils::split_fields_remove_checksum(sentence);

    // GST standard is 9 fields (including header)
    if (fields.size() < 9) {
        return out;
    }

    out.timestamp = gps_utils::parse_nmea_utc_time_to_seconds(fields[1]);
    out.rms_deviation = gps_utils::safe_to_double(fields[2], 0.0);
    out.semi_major_std_dev = gps_utils::safe_to_double(fields[3], 0.0);
    out.semi_minor_std_dev = gps_utils::safe_to_double(fields[4], 0.0);
    out.orientation = gps_utils::safe_to_double(fields[5], 0.0);
    out.lat_std_dev = gps_utils::safe_to_double(fields[6], 0.0);
    out.lon_std_dev = gps_utils::safe_to_double(fields[7], 0.0);
    out.alt_std_dev = gps_utils::safe_to_double(fields[8], 0.0);

    out.is_valid = true;

    return out;
}

bool NMEA_PARSER::is_heading_sentence(const std::string& sentence) const
{
    return sentence.rfind("$GPHDT,", 0) == 0 || sentence.rfind("$GNHDT,", 0) == 0;
}

GPSOrientationStruct NMEA_PARSER::parse_heading(const std::string& sentence) const
{
    GPSOrientationStruct out{};
    out.is_valid = false;

    const std::vector<std::string> fields = gps_utils::split_fields_remove_checksum(sentence);

    if (fields.size() < 2) {
        return out;
    }

    out.timestamp = 0.0;
    out.heading = gps_utils::safe_to_double(fields[1], 0.0);

    out.pitch = 0.0;
    out.roll = 0.0;
    out.heave = 0.0;
    out.speed_kmh = 0.0;
    out.speed_mps = 0.0;
    out.course_over_ground = 0.0;
    out.rate_of_turn = 0.0;

    out.is_valid = !fields[1].empty();

    return out;
}

}  // namespace hemisphere_gnss_v500_driver
