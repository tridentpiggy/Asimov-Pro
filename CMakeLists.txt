cmake_minimum_required(VERSION 3.16)
project(AsimovEditor VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(asimov-editor src/main.cpp)
target_compile_features(asimov-editor PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(asimov-editor PRIVATE /W4 /permissive-)
else()
    target_compile_options(asimov-editor PRIVATE -Wall -Wextra -Wpedantic)
endif()
