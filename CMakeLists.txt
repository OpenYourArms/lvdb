cmake_minimum_required(VERSION 3.12)
project(lvdb LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++11 -pthread")
#for test
enable_testing()

add_subdirectory(src)
add_subdirectory(connect)
# test dir
add_subdirectory(test)

add_executable(lvdb main.cpp)

target_link_libraries(lvdb SrcLibs cntSome)