cmake_minimum_required(VERSION 3.4.1)

project(test)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(test
				test.cpp
				../godiceapi.c)

target_include_directories(test PRIVATE "..")
