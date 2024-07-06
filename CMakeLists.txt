cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 20)

project(Wool)

file(GLOB_RECURSE PROJECT_SOURCES "src/*.cpp")

add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES})

target_link_libraries(${PROJECT_NAME} ${USOCKETS_LIBRARY} curl spdlog)

install(TARGETS ${PROJECT_NAME}
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)

install(DIRECTORY include/
        DESTINATION include/${PROJECT_NAME}
        FILES_MATCHING PATTERN "*.hpp")

install(FILES WoolConfig.cmake DESTINATION lib/cmake/Wool)

add_executable(example examples/basic.cpp)
target_link_libraries(example ${PROJECT_NAME})