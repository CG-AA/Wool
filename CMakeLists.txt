cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 17)

project(Wool)

file(GLOB_RECURSE PROJECT_SOURCES "src/*.cpp")

find_library(USOCKETS_LIBRARY uSockets.a PATHS /usr/lib REQUIRED)
find_package(ZLIB REQUIRED)

add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES})

target_link_libraries(${PROJECT_NAME} ${USOCKETS_LIBRARY})
target_link_libraries(${PROJECT_NAME} ZLIB::ZLIB)

install(TARGETS ${PROJECT_NAME}
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)

install(DIRECTORY include/
        DESTINATION include/${PROJECT_NAME}
        FILES_MATCHING PATTERN "*.h")