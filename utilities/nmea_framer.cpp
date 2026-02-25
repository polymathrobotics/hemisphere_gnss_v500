#include "hemisphere_gnss_v500_driver/utilities/nmea_framer.hpp"


std::vector<std::string> NMEA_FRAMER::on_nmea_frame(std::vector<uint8_t> bytes)
{
    std::lock_guard<std::mutex> lock(bytes_mutex);
    size_t old_size = bytes_accumulator_.size();
    bytes_accumulator_.insert(bytes_accumulator_.end(), bytes.begin(), bytes.end());
    nmea_sentences_.clear();
    this->nmea_sentences_split(old_size);
    return nmea_sentences_;
}

void NMEA_FRAMER::nmea_sentences_split(size_t old_buff_size)
{

    auto start_byte_it = bytes_accumulator_.begin() + old_buff_size;
    auto end_byte_it = bytes_accumulator_.end();

    std::string sentence = "";

    if (start_byte_it > end_byte_it) {
        return;
    }


    // try to optimize this loop if possible
    for (auto it = start_byte_it; it != end_byte_it; ++it) {
        auto byte = *it;

        if (byte == 36) { // '$' character indicates start of a new NMEA sentence
            if (byte_start_)
            {
                // reset and start a new sentence
                sentence = ""; // Ensure the previous partial data is cleared
            } else {
                byte_start_ = true;
            }
        } else if (byte == 13) {
            if (byte_start_) {
                byte_ending_flag = true;
            } else {
                // we got a carriage return without starting a sentence, ignore
            }
        } else if (byte == 10) { // Note: Changed 26 to 10 for Line Feed (\n)
            if (byte_start_ && byte_ending_flag) {
                nmea_sentences_.push_back(sentence);
                sentence = "";
                byte_start_ = false;
                byte_ending_flag = false;
                end_byte_it = it;
            } else {
                // we got a character without properly framing a sentence, ignore
            }
        } else {
            if (byte_start_) {
                sentence += static_cast<char>(byte);
            } else {
                // we got data bytes without starting a sentence, ignore
            }
        }
    }
    // this->nmea_bytes_clean(); // need to figure out a good way to do this
}



void NMEA_FRAMER::nmea_check_sum(std::string sentence)
{
    // implement nmea checksum logic here
}

void NMEA_FRAMER::nmea_bytes_clean()
{
    // implement byte cleaning logic here - remove all bytes up to the last processed end_byte_it
}
