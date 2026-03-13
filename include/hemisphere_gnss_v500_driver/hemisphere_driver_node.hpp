#ifndef HEMISPHERE_DRIVER_NODE_HPP_
#define HEMISPHERE_DRIVER_NODE_HPP_

#include <memory>
#include <string>
#include <functional>
#include <vector>

#include "tcp_client.hpp"
#include "nmea_parser.hpp"
#include "nmea_framer.hpp"

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"
#include "rclcpp_components/register_node_macro.hpp"

#include "sensor_msgs/msg/nav_sat_fix.hpp"
#include "sensor_msgs/msg/imu.hpp"

namespace hemisphere_gnss_v500_driver
{


using CallbackReturn = rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

class HemisphereDriverNode : public rclcpp_lifecycle::LifecycleNode {
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

    // private helpers and variables
    void on_gps_bytes_receive(const std::vector<uint8_t>& bytes);
    void publish_gps_position(const GPSPositionStruct& gps_position_data);
    void publish_gps_orientation(const GPSOrientationStruct& gps_orientation_data);
    // ROS 2 Objects
    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp_lifecycle::LifecyclePublisher<sensor_msgs::msg::NavSatFix>::SharedPtr gps_position_publisher_;
    rclcpp_lifecycle::LifecyclePublisher<sensor_msgs::msg::Imu>::SharedPtr gps_orientation_publisher_;
    sensor_msgs::msg::NavSatFix gps_position_msg_;
    sensor_msgs::msg::Imu gps_orientation_msg_;

    std::unique_ptr<TCP_Client> tcp_client_;
    std::unique_ptr<NMEA_PARSER> nmea_parser_;
    std::unique_ptr<NMEA_FRAMER> nmea_framer_;
    // State variables
    size_t count_;
    std::vector<std::string> nmea_sentences_;
};

} // namespace hemisphere_gnss_v500_driver

#endif // HEMISPHERE_DRIVER_NODE_HPP_
