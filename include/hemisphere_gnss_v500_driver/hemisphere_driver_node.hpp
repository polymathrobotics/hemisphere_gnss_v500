// write the header file for the driver here

#ifndef HEMISPHERE_DRIVER_NODE_HPP_
#define HEMISPHERE_DRIVER_NODE_HPP_

#include <memory>
#include <string>

#include "utilities/udp_socket_driver.hpp"
#include "utilities/nmea_parser.hpp"
#include "utilities/nmea_framer.hpp"

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"
#include "rclcpp_components/register_node_macro.hpp"

#include "sensor_msgs/msg/nav_sat_fix.hpp"
#include "diagnostic_msgs/msg/diagnostic_status.hpp"


namespace hemisphere_gnss_v500_driver
{

class HemisphereDriverNode : public rclcpp::Node {
public:
  // Constructor
  explicit HemisphereDriverNode(const rclcpp::NodeOptions & options);

protected:

  // lifecycle callbacks
    CallbackReturn on_configure(const rclcpp_lifecycle::State & state) override;
    CallbackReturn on_activate(const rclcpp_lifecycle::State & state) override;
    CallbackReturn on_deactivate(const rclcpp_lifecycle::State & state) override;
    CallbackReturn on_cleanup(const rclcpp_lifecycle::State & state) override;
    CallbackReturn on_shutdown(const rclcpp_lifecycle::State & state) override;

private:

    // receive bytes callback from the UDP socket driver

    void on_gps_bytes_receive(const std::vector<uint8_t>& bytes);
    void publish_gps_msg(const GPSDataStruct& gps_data);
    // ROS 2 Objects
    rclcpp::Time::SharedPtr timer_;
    rclcpp::Publisher<sensor_msgs::msg::NavSatFix>::SharedPtr gps_publisher_;
    rclcpp::P
    sensor_msgs::msg::NavSatFix gps_msg_;

    UDP_SOCKET_DRIVER udp_socket_driver_;
    NMEA_PARSER nmea_parser_;
    NMEA_FRAMER nmea_framer_;
    // State variables
    size_t count_;
    std::vector<std::string> nmea_sentences_;
};

} // namespace hemisphere_gnss_v500_driver

#endif // HEMISPHERE_DRIVER_NODE_HPP_
