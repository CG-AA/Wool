cmake_minimum_required (VERSION 3.6)
project(Wool)

# Find required packages
find_package(Threads REQUIRED)
find_path(USOCKETS_INCLUDE_DIR NAMES libusockets.h)
find_library(USOCKETS_LIBRARY NAMES uSockets)

# Define the executable
add_executable(WoolExecutable Wool.cpp)

# Include directories for uSockets
include_directories(${USOCKETS_INCLUDE_DIR})

# Link libraries to the executable
target_link_libraries(WoolExecutable Threads::Threads uWS uv ssl crypto ${USOCKETS_LIBRARY})