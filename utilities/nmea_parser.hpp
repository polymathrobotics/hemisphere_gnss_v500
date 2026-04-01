#pragma once

#include <string>
#include <vector>
#include <variant>
#include <cmath>

#include "gps_utils.hpp"

namespace hemisphere_gnss_v500_driver {

  // 1. Define base structs first
  struct GPSPositionStruct {
    double timestamp;
    double latitude;
    double longitude;
    double altitude;
    int fix_quality;
    int num_satellites;
    double hdop;

    double lat_std_dev;
    double lon_std_dev;
    double alt_std_dev;

    bool is_valid;
  };

  struct GPSCovarianceStruct {
      double timestamp;
      double rms_deviation;
      double semi_major_std_dev;
      double semi_minor_std_dev;
      double orientation;
      double lat_std_dev;
      double lon_std_dev;
      double alt_std_dev;

      bool is_valid;
  };

  struct GPSOrientationStruct {
      double timestamp;
      double heading;
      double pitch;
      double roll;
      double heave;
      double speed_kmh;
      double speed_mps;
      double course_over_ground;
      double rate_of_turn;

      bool is_valid;
  };

  // 2. Define the combined struct AFTER the dependencies are known
  struct NavSatFix {
      GPSPositionStruct position;
      GPSCovarianceStruct covariance;
  };

  // 3. Define the Result type
  using NMEAParseResult = std::variant<
      std::monostate,
      GPSPositionStruct,
      GPSOrientationStruct,
      GPSCovarianceStruct,
      NavSatFix
  >;

  class NMEA_PARSER
  {
  public:
      NMEA_PARSER();
      ~NMEA_PARSER();
      NMEAParseResult on_nmea_parse(const std::string& nmea_sentence);

  private:
      bool is_gga_sentence(const std::string& sentence) const;
      GPSPositionStruct parse_gga(const std::string& sentence) const;

      bool is_gst_sentence(const std::string& sentence) const;
      GPSCovarianceStruct parse_gst(const std::string& sentence) const;

      bool is_heading_sentence(const std::string& sentence) const;
      GPSOrientationStruct parse_heading(const std::string& sentence) const;

      // Class members are fine here because structs are defined above
      GPSPositionStruct last_gga_{};
      GPSCovarianceStruct last_gst_{};
  };
}
