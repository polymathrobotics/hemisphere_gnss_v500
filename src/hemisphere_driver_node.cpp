#include "hemisphere_gnss_v500_driver/hemisphere_driver_node.hpp"

using CallbackReturn = rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

namespace hemisphere_gnss_v500_driver
{

HemisphereDriverNode::HemisphereDriverNode(const rclcpp::NodeOptions & options)
  : rclcpp_lifecycle::LifecycleNode("hemisphere_driver", options) {

    this->declare_parameter<std::string>("ip_address", "192.168.1.100");
    this->declare_parameter<int>("port", 5000);
    this->declare_parameter<int>("timeout_s", 5);
    this->declare_parameter<std::string>("gps_position_topic", "/gps/fix");
    this->declare_parameter<std::string>("gps_orientation_topic", "/gps/orientation/fix");
    this->declare_parameter<int>("buffer_size", 8192);
    this->declare_parameter<std::string>("nmea_command", "$JASC,GPGGA,1");
    this->declare_parameter<std::string>("frame_id", "frame_id");

}

CallbackReturn HemisphereDriverNode::on_configure(const rclcpp_lifecycle::State &) {

  RCLCPP_INFO(get_logger(), "Configured: Node is confgured");
  RCLCPP_INFO(get_logger(), "on_configure: Starting configuration...");

  RCLCPP_INFO(get_logger(), "on_configure: IP Address: %s", this->get_parameter("ip_address").as_string().c_str());
  RCLCPP_INFO(get_logger(), "on_configure: Port: %d", this->get_parameter("port").as_int());
  RCLCPP_INFO(get_logger(), "on_configure: Timeout: %d", this->get_parameter("timeout_s").as_int());
  RCLCPP_INFO(get_logger(), "on_configure: Buffer Size: %d", this->get_parameter("buffer_size").as_int());
  RCLCPP_INFO(get_logger(), "on_configure: NMEA Command: %s", this->get_parameter("nmea_command").as_string().c_str());
  RCLCPP_INFO(get_logger(), "on_configure: GPS Position Topic: %s", this->get_parameter("gps_position_topic").as_string().c_str());
  RCLCPP_INFO(get_logger(), "on_configure: GPS Orientation Topic: %s", this->get_parameter("gps_orientation_topic").as_string().c_str());

  RCLCPP_INFO(get_logger(), "on_configure: Creating publishers...");
  // declare publishers/subscribers below
  gps_position_publisher_ = this->create_publisher<sensor_msgs::msg::NavSatFix>(this->get_parameter("gps_position_topic").as_string(), 10);
  gps_orientation_publisher_ = this->create_publisher<sensor_msgs::msg::Imu>(this->get_parameter("gps_orientation_topic").as_string(), 10);
  tcp_client_ = std::make_unique<TCP_Client>(
        this->get_parameter("ip_address").as_string(),
        this->get_parameter("port").as_int(),
        this->get_parameter("timeout_s").as_int(),
        this->get_parameter("buffer_size").as_int()
    );
  nmea_parser_ = std::make_unique<NMEA_PARSER>();
  nmea_framer_ = std::make_unique<NMEA_FRAMER>();
  return CallbackReturn::SUCCESS;
}


CallbackReturn HemisphereDriverNode::on_activate(const rclcpp_lifecycle::State & state) {
  gps_position_publisher_->on_activate();
  gps_orientation_publisher_->on_activate();
  tcp_client_->loadSocketConfigurations();
  tcp_client_->sendCommand(this->get_parameter("nmea_command").as_string());
  tcp_client_->setBytesCallback(std::bind(&HemisphereDriverNode::on_gps_bytes_receive, this, std::placeholders::_1));
  tcp_client_->run();
  RCLCPP_INFO(get_logger(), "Activated: Node is now processing data.");
  return CallbackReturn::SUCCESS;
}

CallbackReturn HemisphereDriverNode::on_deactivate(const rclcpp_lifecycle::State & state) {
  gps_position_publisher_->on_deactivate();
  gps_orientation_publisher_->on_deactivate();
  tcp_client_->disconnectFromServer();
  RCLCPP_INFO(get_logger(), "Deactivated: Node is paused.");
  return CallbackReturn::SUCCESS;
}

CallbackReturn HemisphereDriverNode::on_cleanup(const rclcpp_lifecycle::State &) {
  timer_.reset();
  gps_position_publisher_.reset();
  gps_orientation_publisher_.reset();
  RCLCPP_INFO(get_logger(), "Cleaning up: Resources released.");
  return CallbackReturn::SUCCESS;
}

CallbackReturn HemisphereDriverNode::on_shutdown(const rclcpp_lifecycle::State & state) {
  RCLCPP_INFO(get_logger(), "Shutting down from state %s", state.label().c_str());
  return CallbackReturn::SUCCESS;
}


void HemisphereDriverNode::on_gps_bytes_receive(const std::vector<uint8_t>& bytes) {

  nmea_sentences_ = nmea_framer_->on_nmea_frame(bytes);
  for (auto nmea_sentence: nmea_sentences_) {
    NMEAParseResult result = nmea_parser_->on_nmea_parse(nmea_sentence);
    if (std::holds_alternative<hemisphere_gnss_v500_driver::GPSPositionStruct>(result)) {
      auto gps_position = std::get<hemisphere_gnss_v500_driver::GPSPositionStruct>(result);
      this->publish_gps_position(gps_position);
    } else if (std::holds_alternative<hemisphere_gnss_v500_driver::GPSOrientationStruct>(result)) {
        auto gps_orientation = std::get<hemisphere_gnss_v500_driver::GPSOrientationStruct>(result);
        this->publish_gps_orientation(gps_orientation);
    } else {
        return;
    }
  }

}

void HemisphereDriverNode::publish_gps_position(const GPSPositionStruct& gps_position_data)
{
  gps_position_msg_.header.stamp = now();
  gps_position_msg_.header.frame_id = "gps_link";
  gps_position_msg_.latitude = gps_position_data.latitude;
  gps_position_msg_.longitude = gps_position_data.longitude;
  gps_position_msg_.altitude = gps_position_data.altitude;

  if (!gps_position_data.is_valid || gps_position_data.fix_quality == 0) {
    gps_position_msg_.status.status = sensor_msgs::msg::NavSatStatus::STATUS_NO_FIX;
  } else if (gps_position_data.fix_quality == 2) {
    gps_position_msg_.status.status = sensor_msgs::msg::NavSatStatus::STATUS_SBAS_FIX;
  } else {
    gps_position_msg_.status.status = sensor_msgs::msg::NavSatStatus::STATUS_FIX;
  }

  gps_position_msg_.status.service = sensor_msgs::msg::NavSatStatus::SERVICE_GPS;

  if (gps_position_data.lat_std_dev > 0.0 &&
      gps_position_data.lon_std_dev > 0.0 &&
      gps_position_data.alt_std_dev > 0.0) {
    gps_position_msg_.position_covariance[0] =
        gps_position_data.lat_std_dev * gps_position_data.lat_std_dev;
    gps_position_msg_.position_covariance[1] = 0.0;
    gps_position_msg_.position_covariance[2] = 0.0;
    gps_position_msg_.position_covariance[3] = 0.0;
    gps_position_msg_.position_covariance[4] =
        gps_position_data.lon_std_dev * gps_position_data.lon_std_dev;
    gps_position_msg_.position_covariance[5] = 0.0;
    gps_position_msg_.position_covariance[6] = 0.0;
    gps_position_msg_.position_covariance[7] = 0.0;
    gps_position_msg_.position_covariance[8] =
        gps_position_data.alt_std_dev * gps_position_data.alt_std_dev;
    gps_position_msg_.position_covariance_type =
        sensor_msgs::msg::NavSatFix::COVARIANCE_TYPE_DIAGONAL_KNOWN;
  } else {
    gps_position_msg_.position_covariance[0] = 0.0;
    gps_position_msg_.position_covariance[1] = 0.0;
    gps_position_msg_.position_covariance[2] = 0.0;
    gps_position_msg_.position_covariance[3] = 0.0;
    gps_position_msg_.position_covariance[4] = 0.0;
    gps_position_msg_.position_covariance[5] = 0.0;
    gps_position_msg_.position_covariance[6] = 0.0;
    gps_position_msg_.position_covariance[7] = 0.0;
    gps_position_msg_.position_covariance[8] = 0.0;
    gps_position_msg_.position_covariance_type =
        sensor_msgs::msg::NavSatFix::COVARIANCE_TYPE_UNKNOWN;
  }
  RCLCPP_INFO(this->get_logger(), "Publishing GPS: Lat: %f, Lon: %f", gps_position_msg_.latitude, gps_position_msg_.longitude);

  gps_position_publisher_->publish(gps_position_msg_);
}

void HemisphereDriverNode::publish_gps_orientation(const GPSOrientationStruct& gps_orientation_data)
{
  // TODO @aarush5 Implement IMU publishing as well soon
  (void)gps_orientation_data;
}


} // namespace hemisphere_gnss_v500_driver


RCLCPP_COMPONENTS_REGISTER_NODE(hemisphere_gnss_v500_driver::HemisphereDriverNode)
