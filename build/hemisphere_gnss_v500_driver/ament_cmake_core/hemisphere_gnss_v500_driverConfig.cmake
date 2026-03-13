# generated from ament/cmake/core/templates/nameConfig.cmake.in

# prevent multiple inclusion
if(_hemisphere_gnss_v500_driver_CONFIG_INCLUDED)
  # ensure to keep the found flag the same
  if(NOT DEFINED hemisphere_gnss_v500_driver_FOUND)
    # explicitly set it to FALSE, otherwise CMake will set it to TRUE
    set(hemisphere_gnss_v500_driver_FOUND FALSE)
  elseif(NOT hemisphere_gnss_v500_driver_FOUND)
    # use separate condition to avoid uninitialized variable warning
    set(hemisphere_gnss_v500_driver_FOUND FALSE)
  endif()
  return()
endif()
set(_hemisphere_gnss_v500_driver_CONFIG_INCLUDED TRUE)

# output package information
if(NOT hemisphere_gnss_v500_driver_FIND_QUIETLY)
  message(STATUS "Found hemisphere_gnss_v500_driver: 0.0.0 (${hemisphere_gnss_v500_driver_DIR})")
endif()

# warn when using a deprecated package
if(NOT "" STREQUAL "")
  set(_msg "Package 'hemisphere_gnss_v500_driver' is deprecated")
  # append custom deprecation text if available
  if(NOT "" STREQUAL "TRUE")
    set(_msg "${_msg} ()")
  endif()
  # optionally quiet the deprecation message
  if(NOT ${hemisphere_gnss_v500_driver_DEPRECATED_QUIET})
    message(DEPRECATION "${_msg}")
  endif()
endif()

# flag package as ament-based to distinguish it after being find_package()-ed
set(hemisphere_gnss_v500_driver_FOUND_AMENT_PACKAGE TRUE)

# include all config extra files
set(_extras "")
foreach(_extra ${_extras})
  include("${hemisphere_gnss_v500_driver_DIR}/${_extra}")
endforeach()
