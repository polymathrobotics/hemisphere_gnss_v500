#include "hemisphere_gnss_v500_driver/utilities/nmea_framer.hpp"


namespace hemisphere_gnss_v500_driver
{

std::vector<std::string> NMEA_FRAMER::on_nmea_frame(std::vector<uint8_t> bytes)
{
    std::lock_guard<std::mutex> lock(bytes_mutex);
    bytes_accumulator_ = bytes;
    nmea_sentences_.clear();
    this->nmea_sentences_split();
    return nmea_sentences_;
}

void NMEA_FRAMER::nmea_sentences_split()
{

    auto start_byte_it = bytes_accumulator_.begin();
    auto end_byte_it = bytes_accumulator_.end();

    if (start_byte_it > end_byte_it) {
        return;
    }

    for (auto it = start_byte_it; it != end_byte_it; ++it) {
        auto byte = *it;

        if (byte == 36) { // '$' character indicates start of a new NMEA sentence

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
                byte_ending_flag = false;
            } else {
                // we got data bytes without starting a sentence, ignore
            }
        }
    }
}



bool NMEA_FRAMER::nmea_check_sum(const std::string& sentence)
{
    if (sentence.empty() || sentence.front() != '$') {
        return false;
    }

    auto star_it = std::find(sentence.begin(), sentence.end(), '*');
    if (star_it == sentence.end()) {
        return false;
    }

    if (std::distance(star_it, sentence.end()) < 3) {
        return false;
    }

    uint8_t computed_checksum = 0;
    for (auto it = sentence.begin() + 1; it != star_it; ++it) {
        computed_checksum ^= static_cast<uint8_t>(*it);
    }

    std::string checksum_str(star_it + 1, star_it + 3);

    char* end_ptr = nullptr;
    long transmitted_checksum = std::strtol(checksum_str.c_str(), &end_ptr, 16);

    if (end_ptr != checksum_str.c_str() + 2) {
        return false;
    }

    return computed_checksum == static_cast<uint8_t>(transmitted_checksum);
}

}
