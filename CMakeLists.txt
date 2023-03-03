cmake_minimum_required(VERSION 3.24)
project(networking)

set(CMAKE_CXX_STANDARD 17)

add_executable(experiment-1-server server.cpp)
add_executable(experiment-1-client client.cpp)
