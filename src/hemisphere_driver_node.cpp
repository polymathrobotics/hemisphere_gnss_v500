#include "hemisphere_gnss_v500_driver/include/hemisphere_gnss_v500_driver/hemisphere_driver_node.hpp"

// Shorthand for the transition return type
using CallbackReturn = rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

namespace hemisphere_gnss_v500_driver
{

HemisphereDriverNode::HemisphereDriverNode(const rclcpp::NodeOptions & options)
  : rclcpp_lifecycle::LifecycleNode("hemisphere_driver_node", options) {

    this->declare_parameter<std::string>("ip_address", "192.168.1.100");
    this->declare_parameter<int>("port", 5005);
    this->declare_parameter<int>("timeout_s", 5);
    this->declare_parameter<std::string>("gps_topic", "/gps/fix");
    this->declare_parameter<std::string>("diagnostics_topic", "/gps/rtk/diagnostics");
    this->declare_parameter<int>("buffer_size", 8192);

}

CallbackReturn on_configure(const rclcpp_lifecycle::State &) {

  // declare publishers/subscribers below
  gps_publisher_ = this->create_publisher<sensor_msgs::msg::NavSatFix>(this->get_parameter("gps_topic").as_string(), 10);
  diagnostics_publisher = this->create_publisher<diagnostics_msgs::msg::DiagnosticStatus>(this->get_parameter("diagnostics_topic").as_string(), 10);
  udp_socket_driver_ = UDP_SOCKET_DRIVER(this->get_parameter("ip_address").as_string(), this->get_parameter("port").as_int(), this->get_parameter("timeout_s").as_int(), this->get_parameter("buffer_size").as_int());
  nmea_parser_ = NMEA_PARSER(); // need to pass in the bytes somehow
  nmea_framer_ = NMEA_FRAMER();
  return CallbackReturn::SUCCESS;
}


CallbackReturn on_activate(const rclcpp_lifecycle::State & state) {
  gps_publisher_->on_activate();
  udp_socket_driver_.loadSocketConfigurations();
  udp_socket_driver_.setBytesCallback(std::bind(&HemisphereDriverNode::on_gps_bytes_receive, this));
  udp_socket_driver_.run();
  RCLCPP_INFO(get_logger(), "Activated: Node is now processing data.");
  return CallbackReturn::SUCCESS;
}

CallbackReturn on_deactivate(const rclcpp_lifecycle::State & state) {
  gps_publisher_->on_deactivate();
  udp_socket_driver_.shutdownUDPSocket();
  RCLCPP_INFO(get_logger(), "Deactivated: Node is paused.");
  return CallbackReturn::SUCCESS;
}

CallbackReturn on_cleanup(const rclcpp_lifecycle::State &) {
  timer_.reset();
  gps_publisher_.reset();
  RCLCPP_INFO(get_logger(), "Cleaning up: Resources released.");
  return CallbackReturn::SUCCESS;
}

CallbackReturn on_shutdown(const rclcpp_lifecycle::State & state) {
  RCLCPP_INFO(get_logger(), "Shutting down from state %s", state.label().c_str());
  return CallbackReturn::SUCCESS;
}

} // namespace hemisphere_gnss_v500_driver


// need to figure out the input type somehow
void on_gps_bytes_receive(const std::vector<uint8_t>& bytes) {

  RCLCPP_INFO(get_logger(), "Received %zu bytes", bytes.size());
  nmea_sentences_ = nmea_framer_.on_nmea_frame(bytes);
  for (auto nmea_sentence: nmea_sentences) {
    GPSDataStruct gps_data = nmea_parser_.on_nmea_parse(nmea_sentence);
    this->publish_gps_msg(gps_data);
    this->publish_rtk_diagnostics_msg(const GPSDataStruct& gps_data);
  }

}

void publish_rtk_diagnostics_msg(const GPSDataStruct& gps_data) {

  // empty placeholder for now
  // parse the diagnostics message somehow

}

void publish_gps_msg(const GPSDataStruct& gps_data) {
  // populate gps_msg_ with the relevant fields from gps_data
  // gps_msg_.latitude = gps_data.latitude;
  // gps_msg_.longitude = gps_data.longitude;
  // gps_msg_.altitude = gps_data.altitude;
  // etc.

  gps_publisher_->publish(gps_msg_);
}

RCLCPP_COMPONENTS_REGISTER_NODE(hemisphere_gnss_v500_driver::HemisphereDriverNode)
