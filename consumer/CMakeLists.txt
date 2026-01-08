cmake_minimum_required(VERSION 3.15)
project(minisql_consumer LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(mini-sql CONFIG REQUIRED)

add_executable(consumer main.cpp)
target_link_libraries(consumer PRIVATE minisql::minisql)