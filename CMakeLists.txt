cmake_minimum_required(VERSION 3.15)

project(json)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Ofast -Wfatal-errors -D_MWAITXINTRIN_H_INCLUDED")
set(CMAKE_CXX_STANDARD 20)

add_executable(tree
        main.cpp
        json.h
)

add_executable(json
        json.h
        json.cpp
)

