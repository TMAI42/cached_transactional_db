cmake_minimum_required(VERSION 3.26)
project(cashed_transactional_db)

set(CMAKE_CXX_STANDARD 23)

include_directories(src/includes)

add_executable(cashed_transactional_db src/main.cpp)
