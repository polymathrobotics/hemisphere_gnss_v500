#pragma once

#include <string>
#include <vector>
#include <variant>

#include "gps_utils.hpp"



namespace hemisphere_gnss_v500_driver {

 struct GPSPositionStruct {
  // Timing
    double timestamp;           // Unix/UTC time from the sensor

    // Coordinates
    double latitude;            // Decimal degrees
    double longitude;           // Decimal degrees
    double altitude;            // Meters (Above WGS 84 Ellipsoid)

    // Accuracy & Quality
    int fix_quality;            // 0=Invalid, 1=GPS, 2=DGPS, 4=RTK, 5=Float RTK
    int num_satellites;         // Number of satellites used in solution
    double hdop;                // Horizontal Dilution of Precision

    // Covariance (Uncertainty)
    double lat_std_dev;         // Standard deviation of latitude (meters)
    double lon_std_dev;         // Standard deviation of longitude (meters)
    double alt_std_dev;         // Standard deviation of altitude (meters)

    bool is_valid;              // Flag to ensure data isn't stale or corrupted
  };

  struct GPSCovarianceStruct {
    // Timing
    double timestamp;           // UTC time from the $GPGST sentence

    // Range Statistics
    double rms_deviation;       // Total RMS standard deviation of range inputs

    // Error Ellipse (Calculated by the V500)
    double major_axis_std_dev;  // Standard deviation of semi-major axis (meters)
    double minor_axis_std_dev;  // Standard deviation of semi-minor axis (meters)
    double major_axis_orientation; // Orientation of semi-major axis (degrees from True North)

    // Axis-Specific Standard Deviations (The ones you need for NavSatFix)
    double lat_std_dev;         // Standard deviation of latitude error (meters)
    double lon_std_dev;         // Standard deviation of longitude error (meters)
    double alt_std_dev;         // Standard deviation of altitude error (meters)

    bool is_valid;              // True if parsing was successful and checksum passed
  };

  struct GPSOrientationStruct {
      // Timing
      double timestamp;

      // Orientation (Attitude)
      double heading;             // True Heading in degrees (0 to 359.9)
      double pitch;               // Degrees (Up/Down tilt)
      double roll;                // Degrees (Side-to-side tilt)
      double heave;               // Vertical displacement (meters) - common in Marine

      // Motion (Velocity)
      double speed_kmh;           // Speed over ground (SOG)
      double speed_mps;           // Speed over ground converted to meters/second
      double course_over_ground;  // Actual travel direction (may differ from heading)
      double rate_of_turn;        // Degrees per minute (from the V500 gyro)

      bool is_valid;
  };

  using NMEAParseResult = std::variant<std::monostate, GPSPositionStruct, GPSCovarianceStruct, GPSOrientationStruct>;
  class NMEA_PARSER
  {
  public:
      // default constructor
      NMEA_PARSER();
      ~NMEA_PARSER();
      NMEAParseResult on_nmea_parse(const std::string& nmea_sentence);

  private:
      bool is_gga_sentence(const std::string& sentence) const;
      GPSPositionStruct parse_gga(const std::string& sentence) const;
      bool is_gpgst_sentence(const std::string& sentence) const;
      GPSCovarianceStruct parse_gpgst(const std::string& sentence) const;
      bool is_heading_sentence(const std::string& sentence) const;
      GPSOrientationStruct parse_heading(const std::string& sentence) const;


  };
}
