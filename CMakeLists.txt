# cmake version
cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# Project name
project(ccdc)

# Build type
set(CMAKE_BUILD_TYPE "Debug")
#set(CMAKE_BUILD_TYPE "Release")

# C++ 11
set(CMAKE_CXX_FLAGS "-std=c++11")

# Build output path
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
# Libraries path
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
# king
#set(OpenCV_DIR "/usr/local/opencv3.2/share/OpenCV")
# OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)


# Add library
add_library(LaneLineDet_LIB SHARED src/LaneLineDet.cpp	)
target_link_libraries(LaneLineDet_LIB
	${OpenCV_LIBS}
	)


# Add executable
add_executable(lane_line_detection src/main.cpp)
target_link_libraries(lane_line_detection
	${OpenCV_LIBS}
    LaneLineDet_LIB
	)

# add executable
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/tools)
add_executable(get_img_pts tools/GetimgPoints.cpp)
target_link_libraries(get_img_pts ${OpenCV_LIBS})

add_executable(color_filter tools/ColorFilter.cpp)
target_link_libraries(color_filter ${OpenCV_LIBS})
