# 1. Minimum version 3.22 for Ubuntu 22.04 (Humble)
cmake_minimum_required(VERSION 3.22)
project(hemisphere_gnss_v500_driver)

# 2. Use C++17
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

# 3. Compiler Settings
# Note: "-Wl,--no-undefined" removed to match original working build behavior
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic -Werror=switch)
endif()

# 4. Find dependencies using ament_cmake_auto (Style Guide requirement)
find_package(ament_cmake_auto REQUIRED)
ament_auto_find_build_dependencies()

# 5. Gather Utility Sources (Reverting to your working GLOB logic)
file(GLOB_RECURSE UTIL_SOURCES "utilities/*.cpp")

# 6. Create the Component Library
# Using the name 'hemisphere_component' from your original file
add_library(hemisphere_component SHARED
  src/hemisphere_driver_node.cpp
  ${UTIL_SOURCES}
)

# Set include directories
target_include_directories(hemisphere_component PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/utilities>"
  "$<INSTALL_INTERFACE:include>"
)

# 7. Modern Link against dependencies
# Using ${package_TARGETS} for messages as required by the guide
target_link_libraries(hemisphere_component
  PUBLIC
    rclcpp::rclcpp
    rclcpp_lifecycle::rclcpp_lifecycle
    rclcpp_components::component
    ${sensor_msgs_TARGETS}
)

# 8. Register the Component
rclcpp_components_register_node(hemisphere_component
  PLUGIN "hemisphere_gnss_v500_driver::HemisphereDriverNode"
  EXECUTABLE hemisphere_driver_node
)

# 9. Install includes
install(
  DIRECTORY include/
  DESTINATION include
)

# 10. Install launch and config
install(
  DIRECTORY launch config
  DESTINATION share/${PROJECT_NAME}
)

# 11. Install the library targets (With Export for Style Guide compliance)
install(TARGETS
  hemisphere_component
  EXPORT hemisphere_component_TARGETS
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
)

# 12. Export targets for downstream use
install(
  EXPORT hemisphere_component_TARGETS
  NAMESPACE ${PROJECT_NAME}::
  DESTINATION share/${PROJECT_NAME}/cmake
)

# 13. Testing
if(BUILD_TESTING)
  ament_auto_find_test_dependencies()
endif()

# 14. Ament Export and Package
ament_export_targets(hemisphere_component_TARGETS HAS_LIBRARY_TARGET)
ament_package()
