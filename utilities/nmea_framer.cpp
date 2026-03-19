#include "nmea_framer.hpp"


namespace hemisphere_gnss_v500_driver
{

std::vector<std::string> NMEA_FRAMER::on_nmea_frame(std::vector<uint8_t> bytes)
{
    std::lock_guard<std::mutex> lock(bytes_mutex);
    nmea_sentences_.clear();
    this->nmea_sentences_split(bytes);
    return nmea_sentences_;
}

void NMEA_FRAMER::nmea_sentences_split(std::vector<uint8_t> bytes)
{

    auto start_byte_it = bytes.begin();
    auto end_byte_it = bytes.end();

    if (start_byte_it > end_byte_it) {
        return;
    }

    for (auto it = start_byte_it; it != end_byte_it; ++it) {
        auto byte = *it;

        if (byte == 36) {
            current_sentence = "$";
            byte_start_ = true;
            byte_ending_flag = false;

        } else if (byte == 13) {
            if (byte_start_) {
                byte_ending_flag = true;
            } else {
                continue;
            }
        } else if (byte == 10) {
            if (byte_start_ && byte_ending_flag) {
                if (nmea_check_sum(current_sentence)) {
                    nmea_sentences_.push_back(current_sentence);
                }
                current_sentence = "";
                byte_start_ = false;
                byte_ending_flag = false;
            } else {
                continue;
            }
        } else {
            if (byte_start_) {
                current_sentence += static_cast<char>(byte);
                byte_ending_flag = false;
            } else {
                continue;
            }
        }
    }
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
