cmake_minimum_required(VERSION 3.15)
project(tree)

set(CMAKE_CXX_STANDARD 20)

add_executable(tree
        main.cpp
        json.h
)

add_executable(test
        json.h
        json.cpp)

