cmake_minimum_required(VERSION 3.8)
project(hemisphere_gnss_v500_driver)

# 1. Find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(sensor_msgs REQUIRED)

# 2. Gather Utility Sources
# Ensure your .cpp files for NMEA_PARSER/FRAMER are actually in this folder
file(GLOB_RECURSE UTIL_SOURCES "utilities/*.cpp")

# 3. Create the Component Library
# ROS 2 Components MUST be SHARED libraries
add_library(hemisphere_component SHARED
  src/hemisphere_driver_node.cpp
  ${UTIL_SOURCES}
)

# 4. Include Directories
target_include_directories(hemisphere_component PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/utilities>
  $<INSTALL_INTERFACE:include>
)

# 5. Dependencies
# This handles the linking for all ROS 2 headers and libraries
ament_target_dependencies(hemisphere_component
  rclcpp
  rclcpp_lifecycle
  rclcpp_components
  sensor_msgs
)

# 6. Register the Component
# This macro tells ROS 2 how to load your node as a plugin.
# It ALSO generates a standalone executable automatically so you don't need 'add_executable'
rclcpp_components_register_node(hemisphere_component
  PLUGIN "hemisphere_gnss_v500_driver::HemisphereDriverNode"
  EXECUTABLE hemisphere_driver_node
)

# 7. Install
install(TARGETS hemisphere_component
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

# 8. Export and Ament
ament_package()
