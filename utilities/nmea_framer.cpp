#include "nmea_framer.hpp"


namespace hemisphere_gnss_v500_driver
{

std::vector<std::string> NMEA_FRAMER::on_nmea_frame(std::vector<uint8_t> bytes)
{
    std::lock_guard<std::mutex> lock(bytes_mutex);
    RCLCPP_INFO(rclcpp::get_logger("nmea_framer"), "ARRIVED AT ON NMEA FRAMER");
    std::stringstream ss;
    for (uint8_t b : bytes) {
        // This formats each byte as a 2-digit Hex (e.g., 0x24 for '$')
        ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(b) << " ";
    }

    RCLCPP_INFO(rclcpp::get_logger("nmea_framer"), "Raw Hex: [ %s]", ss.str().c_str());

    nmea_sentences_.clear();
    this->nmea_sentences_split(bytes);
    return nmea_sentences_;
}

void NMEA_FRAMER::nmea_sentences_split(std::vector<uint8_t> bytes)
{

    auto start_byte_it = bytes.begin();
    auto end_byte_it = bytes.end();
    RCLCPP_INFO(rclcpp::get_logger("nmea_framer"), "BEGINNING OF LOOP");
    if (start_byte_it > end_byte_it) {
        return;
    }

    for (auto it = start_byte_it; it != end_byte_it; ++it) {
        auto byte = *it;

        if (byte == 36) { // '$' character indicates start of a new NMEA sentence
             RCLCPP_INFO(rclcpp::get_logger("nmea_framer"), "reached beginning");
            current_sentence = "$";
            byte_start_ = true;
            byte_ending_flag = false;

        } else if (byte == 13) {
            if (byte_start_) {
                byte_ending_flag = true;
            } else {
                // we got a carriage return without starting a sentence, ignore
            }
        } else if (byte == 10) { // Note: Changed 26 to 10 for Line Feed (\n)
            if (byte_start_ && byte_ending_flag) {
                if (nmea_check_sum(current_sentence)) {
                    nmea_sentences_.push_back(current_sentence);
                }
                current_sentence = "";
                byte_start_ = false;
                byte_ending_flag = false;
            } else {
                // we got a character without properly framing a sentence, ignore
            }
        } else {
            if (byte_start_) {
                current_sentence += static_cast<char>(byte);
                RCLCPP_INFO(rclcpp::get_logger("nmea_framer"), "Appending byte");
                byte_ending_flag = false;
            } else {
                // we got data bytes without starting a sentence, ignore
            }
        }
    }
    RCLCPP_INFO(rclcpp::get_logger("nmea_framer"), "REACHED THE END OF BYTES");
}



bool NMEA_FRAMER::nmea_check_sum(const std::string& sentence)
{
    // 1. Basic Validation
    if (sentence.size() < 4 || sentence.front() != '$') {
        return false;
    }

    // 2. Find the asterisk
    size_t star_pos = sentence.find('*');
    if (star_pos == std::string::npos || star_pos + 2 >= sentence.size()) {
        return false;
    }

    // 3. Calculate XOR sum of everything BETWEEN '$' and '*'
    uint8_t computed_checksum = 0;
    for (size_t i = 1; i < star_pos; ++i) {
        computed_checksum ^= static_cast<uint8_t>(sentence[i]);
    }

    // 4. Extract exactly the 2 hex characters following the '*'
    std::string hex_str = sentence.substr(star_pos + 1, 2);

    // 5. Convert Hex String to Integer safely
    uint32_t transmitted_checksum = 0;
    try {
        transmitted_checksum = std::stoul(hex_str, nullptr, 16);
    } catch (const std::exception& e) {
        RCLCPP_ERROR(rclcpp::get_logger("nmea_framer"),
                     "Checksum: Hex conversion failed for '%s'", hex_str.c_str());
        return false;
    }

    // 6. Compare
    bool match = (computed_checksum == static_cast<uint8_t>(transmitted_checksum));

    if (!match) {
        RCLCPP_ERROR(rclcpp::get_logger("nmea_framer"),
                     "Checksum MISMATCH! Calc: %02X, Sent: %02X | Sentence: %s",
                     computed_checksum, transmitted_checksum, sentence.c_str());
    }

    return match;
}

}
