cmake_minimum_required(VERSION 3.10)
project(dinic_maxflow_benchmark)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(maxflow_benchmark src/main.cpp)
