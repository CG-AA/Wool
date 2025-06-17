cmake_minimum_required(VERSION 3.5)
project(Wool)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DSPDLOG_HEADER_ONLY)

include_directories(third_party/spdlog/include)

add_executable(wool main.cpp)
