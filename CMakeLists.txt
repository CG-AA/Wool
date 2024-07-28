option(B_EX "Build the example executable" OFF)
option(B_LIB "Build the library" ON)

cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 17)

project(Wool)

find_package(CURL REQUIRED)
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)
find_package(WebSocketPP REQUIRED)
find_package(OpenSSL REQUIRED)

if(B_LIB)

	file(GLOB_RECURSE PROJECT_SOURCES "src/*.cpp")

	add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES})

	target_link_libraries(${PROJECT_NAME} 
		CURL::libcurl 
		spdlog::spdlog 
		fmt::fmt 
		OpenSSL::SSL 
		Boost::system
	)

	target_include_directories(${PROJECT_NAME} PRIVATE ${WEVSOCKETPP_INCLUDE_DIR})

	include(GNUInstallDirs)

	install(TARGETS ${PROJECT_NAME}
			ARCHIVE DESTINATION lib
			LIBRARY DESTINATION lib
			RUNTIME DESTINATION bin)

	install(DIRECTORY include/
			DESTINATION include/${PROJECT_NAME}
			FILES_MATCHING PATTERN "*.hpp")

	install(FILES WoolConfig.cmake DESTINATION lib/cmake/Wool)
endif()

if(B_EX)
	add_executable(example examples/basic.cpp)
	target_link_libraries(example ${PROJECT_NAME})
	add_executable(voice examples/voice.cpp)
	target_link_libraries(voice ${PROJECT_NAME})
endif()