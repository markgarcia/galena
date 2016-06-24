project(galena)
cmake_minimum_required(VERSION 2.8)

if(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -pedantic")
endif()

find_package(Boost 1.60 REQUIRED COMPONENTS program_options)
include_directories(${Boost_INCLUDE_DIRS})

include_directories(include)

set(SRC_LIST
    include/galena/application.h

    include/galena/renderer.h
    src/renderer.cpp

    include/galena/window.h
    include/galena/window/window_event.h
    src/window.cpp)

if(WIN32)
    add_definitions(-DUNICODE)

    set(SRC_LIST
        ${SRC_LIST}

        src/application.cpp

        include/galena/renderer/dx11/dx11_renderer.h
        include/galena/renderer/dx11/impl/com_ptr.h
        src/renderer/dx11/dx11_renderer.cpp

        include/galena/window/win32/event_identifier.h
        include/galena/window/win32/types.h
        include/galena/window/win32/window_event.h
        include/galena/window/win32/window_impl.h
        src/window/win32/window_impl.cpp
        src/window/win32/event_identifier.cpp)
endif()

add_library(galena ${SRC_LIST})

if(WIN32)
    target_link_libraries(galena D3D11)
endif()

add_subdirectory(tests)
add_subdirectory(tools/cpp-shader-compiler)
