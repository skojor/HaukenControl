# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\HaukenControl_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\HaukenControl_autogen.dir\\ParseCache.txt"
  "HaukenControl_autogen"
  )
endif()
