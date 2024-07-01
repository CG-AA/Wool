cmake_minimum_required(VERSION 3.6)
project(Wool)

# Find required packages
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)

find_library(USOCKETS_LIBRARY NAMES uSockets.a PATHS /usr/lib NO_DEFAULT_PATH)
find_path(USOCKETS_INCLUDE_DIR libusockets.h PATHS /usr/local/include/ NO_DEFAULT_PATH)

set(SOURCE_FILES
    src/Wool.cpp
)

# Create the Wool static library
add_library(Wool STATIC ${SOURCE_FILES})

# Set the include directories for WoolLib
target_include_directories(Wool PUBLIC ${USOCKETS_INCLUDE_DIR})

# Link the Wool library with the required libraries
target_link_libraries(Wool Threads::Threads sodium fmt ssl ZLIB uWS ${USOCKETS_LIBRARY})

# Use GNUInstallDirs for installation directories
include(GNUInstallDirs)
install(TARGETS Wool
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Wool)