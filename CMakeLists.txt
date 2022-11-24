cmake_minimum_required(VERSION 3.23)
project(computerlab6)

set(CMAKE_CXX_STANDARD 20)

add_executable(computerlab6 main.cpp)
target_link_libraries(computerlab6 usb-1.0)