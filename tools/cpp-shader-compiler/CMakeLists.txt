project(galena-cpp-shader-compiler)
cmake_minimum_required(VERSION 2.8)

set(SRC_LIST
    src/main.cpp)

include_directories(extlibs/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/extlibs/lib)

add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} clang)
target_link_libraries(${PROJECT_NAME} Boost::program_options)
