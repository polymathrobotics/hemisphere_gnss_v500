#pragma once


#include <vector>
#include <cstdint>
#include <string>
#include <mutex>
#include <iostream>
#include <map>
// high level class overview

// 1. take in an array of bytes from the udp_socket_driver
// 2. append the bytes in a global buffer
// 3. every time new bytes come in, i go through the global bytes and parse all sentences
// 4. pass the sentences on to nmea parser


namespace hemisphere_gnss_v500_driver
{

    class NMEA_FRAMER
    {

        public:
            NMEA_FRAMER() : byte_start_(false), byte_ending_flag(false), middle_of_sentence_flag_(false) {};
            std::vector<std::string> on_nmea_frame(std::vector<uint8_t> bytes);

        private:

            void nmea_sentences_split(size_t current_buff_size);
            void nmea_bytes_clean();
            void nmea_check_sum(std::string sentence);
            // put all private helper functions below
            std::mutex bytes_mutex;
            std::vector<uint8_t> bytes_accumulator_;
            std::vector<std::string> nmea_sentences_;
            bool byte_start_;
            bool byte_ending_flag;
            bool middle_of_sentence_flag_;
    };

}
