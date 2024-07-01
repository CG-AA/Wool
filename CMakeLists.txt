cmake_minimum_required (VERSION 3.6)
project(Wool)

# Find required packages
find_package(Threads REQUIRED)

# Define the executable
add_executable(WoolExecutable Wool.cpp)

# Include directories for uSockets
include_directories(${USOCKETS_INCLUDE_DIR})

# Link libraries to the executable
target_link_libraries(WoolExecutable Threads::Threads uWS uv ssl crypto uSockets)