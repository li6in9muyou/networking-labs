cmake_minimum_required(VERSION 3.24)
project(networking)

set(CMAKE_CXX_STANDARD 17)

add_executable(experiment-1-server server.cpp)
target_link_libraries(experiment-1-server PRIVATE Ws2_32)

add_executable(experiment-1-client client.cpp)
target_link_libraries(experiment-1-client PRIVATE Ws2_32 Mswsock AdvApi32)
