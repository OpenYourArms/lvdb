cmake_minimum_required(VERSION 3.12)
project(lvdb LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
#for test
enable_testing()

add_subdirectory(src)
add_subdirectory(connect)
# test dir
add_subdirectory(test)

add_executable(lvdb main.cpp)

target_link_libraries(lvdb SrcLibs cntSome)