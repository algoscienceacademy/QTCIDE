# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "")
  file(REMOVE_RECURSE
  "CMakeFiles\\QTCIDE_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\QTCIDE_autogen.dir\\ParseCache.txt"
  "QTCIDE_autogen"
  )
endif()
