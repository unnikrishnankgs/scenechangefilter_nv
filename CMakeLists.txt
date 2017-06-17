cmake_minimum_required(VERSION 2.8)
project( displayimg )
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )
add_executable( displayimg displayimg.cpp )
target_link_libraries( displayimg ${OpenCV_LIBS} )

