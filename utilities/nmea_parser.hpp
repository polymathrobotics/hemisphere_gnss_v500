#pragma once

#include <string>
#include <vector>


struct GPSDataStruct {
  double latitude;
  double longitude;
  double altitude;
  // add other relevant fields as needed
};

// state machine for nmea parser
// High Level Design for it:
// 1.

enum class PARSE_STATE {
  START,
  WORD,
  CHECKSUM,
  END
};

namespace hemisphere_gnss_v500_driver
{

    class NMEA_PARSER
    {

        public:
            NMEA_PARSER();

            // logic for parsing the

        private:
            int field_index_;
            PARSE_STATE current_parse_state_;
    };
}
