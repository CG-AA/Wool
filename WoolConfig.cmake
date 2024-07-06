cmake_minimum_required(VERSION 3.15)

find_library(USOCKETS_LIBRARY uSockets.a PATHS /usr/local/lib REQUIRED)
find_package(ZLIB REQUIRED)
find_package(fmt REQUIRED)

add_library(Wool::Wool STATIC IMPORTED)

set_target_properties(Wool::Wool PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/include/Wool"
    IMPORTED_LOCATION "${CMAKE_INSTALL_PREFIX}/lib/libWool.a"
)

set_property(TARGET Wool::Wool PROPERTY INTERFACE_LINK_LIBRARIES ${USOCKETS_LIBRARY} ZLIB::ZLIB fmt::fmt sodium)