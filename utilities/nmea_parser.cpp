// Need to go through each nmea sentene, extract words, etc. Can do the following:
// Pseudocode:
// 1. states are a follows: START, WORD, CHECKSUM, END




#include "hemisphere_gnss_v500_driver/utilities/nmea_parser.hpp"


NMEA_PARSER::NMEA_PARSER()
{
    current_parse_state_ = PARSE_STATE::START;
    // include some very basic logic for initializing some stuff
}

void NMEA_PARSER::construct_gps_message(std::string nmea_sentence) {

    // removing the checksum at the end for now
    auto it = std::find(nmea_sentence.begin(), nmea_sentence.end(), '*');
    if (it != nmea_sentence.end()) {
        nmea_sentence.erase(it, nmea_sentence.end());
    }

    // parse the nmea sentence and populate the gps data struct

}

void NMEA_PARSER::on_nmea_parse(std::string nmea_sentence) {
    // ADD delimiter logic here to split up sentences into seperate chars
    // HIGH LEVEL PSEUDOCODE
    // 1. Receive the
    bool nmea_sentence_unchecked = false;
    std::string word = "";
    for (auto character : nmea_sentence)
    {
        if (character == ",")
        {
            // Call a function to parse the word
            word = "";
        } else {
            word += character;
        }
    }
}


void NMEA_PARSER::parse_word(std::string word) {


}
