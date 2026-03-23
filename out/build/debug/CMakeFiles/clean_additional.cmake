# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "City-Navigation-System\\CMakeFiles\\City-Navigation-System_autogen.dir\\AutogenUsed.txt"
  "City-Navigation-System\\CMakeFiles\\City-Navigation-System_autogen.dir\\ParseCache.txt"
  "City-Navigation-System\\City-Navigation-System_autogen"
  )
endif()
