project(galena)
cmake_minimum_required(VERSION 2.8)

if(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -pedantic")
endif()

find_package(Boost 1.60 REQUIRED COMPONENTS program_options)
include_directories(${Boost_INCLUDE_DIRS})

add_subdirectory(tools/cpp-shader-compiler)
