cmake_minimum_required(VERSION 3.24)
project(networking)

set(CMAKE_CXX_STANDARD 17)

add_executable(experiment-2-server conc-server.cpp)
target_link_libraries(experiment-2-server PRIVATE Ws2_32)

add_executable(experiment-2-libuv-server ./libuv-server.cpp)
target_include_directories(experiment-2-libuv-server PRIVATE ./libuv/include)

add_subdirectory(./libuv)
target_link_libraries(experiment-2-libuv-server PRIVATE uv_a)
