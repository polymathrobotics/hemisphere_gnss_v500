#pragma once


#include <vector>
#include <cstdint>
#include <string>
#include <mutex>
#include <iostream>
#include <map>

namespace hemisphere_gnss_v500_driver
{

    class NMEA_FRAMER
    {

        public:
            NMEA_FRAMER() : byte_start_(false), byte_ending_flag(false), current_sentence("") {};
            std::vector<std::string> on_nmea_frame(std::vector<uint8_t> bytes);

        private:

            void nmea_sentences_split();
            bool nmea_check_sum(const std::string& sentence);
            // put all private helper functions below
            std::mutex bytes_mutex;
            std::vector<uint8_t> bytes_accumulator_;
            std::vector<std::string> nmea_sentences_;
            std::string current_sentence;
            bool byte_start_;
            bool byte_ending_flag;
    };

}
