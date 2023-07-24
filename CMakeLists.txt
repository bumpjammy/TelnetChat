cmake_minimum_required(VERSION 3.25)
project(TelnetChat)

set(CMAKE_CXX_STANDARD 17)

add_executable(TelnetChat src/main.cpp src/server.cpp src/handle.cpp src/exception.cpp include/client_handler.hpp include/exception.hpp include/handle.hpp include/server.hpp src/client_handler.cpp)
