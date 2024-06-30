cmake_minimum_required (VERSION 3.6)
project(Wool)

# Find required packages
find_package(Threads REQUIRED)
# Add find_package or add_subdirectory for uWS, uv, ssl, and crypto as necessary

# Define the executable
add_executable(WoolExecutable Wool.cpp) # Changed target name to WoolExecutable

# Link libraries to the executable
# Ensure that the libraries uWS, uv, ssl, and crypto are correctly found or added before this step
target_link_libraries(WoolExecutable Threads::Threads uWS uv ssl crypto uSockets)