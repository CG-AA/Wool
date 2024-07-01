cmake_minimum_required(VERSION 3.6)
project(Wool)

# Find required packages
find_package(Threads REQUIRED)

set(SOURCE_FILES
    src/Wool.cpp
)

# Create the Wool static library
add_library(Wool STATIC ${SOURCE_FILES})

# Set the include directories for WoolLib
target_include_directories(Wool PUBLIC ${USOCKETS_INCLUDE_DIR})

# Link the Wool library with the required libraries
target_link_libraries(Wool Threads::Threads sodium fmt ssl ZLIB uSockets uWS)

# Use GNUInstallDirs for installation directories
include(GNUInstallDirs)
install(TARGETS Wool
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Wool)