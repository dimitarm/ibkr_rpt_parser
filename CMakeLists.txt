cmake_minimum_required(VERSION 3.15)
project(parser)

set(CMAKE_CXX_STANDARD 17)

# Set the build output location to a common directory
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_DEBUG_POSTFIX d)


add_subdirectory(OpenXLSX)

add_executable(parser main.cpp)
set_target_properties(parser PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

target_link_libraries(parser OpenXLSX::OpenXLSX)
target_include_directories(parser PRIVATE /usr/local/include)