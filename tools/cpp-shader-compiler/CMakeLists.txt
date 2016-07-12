project(galena-cpp-shader-compiler)
cmake_minimum_required(VERSION 2.8)

get_cmake_property(VAR LLVM_DIR)
if(NOT LLVM_DIR)
    message(SEND_ERROR "LLVM_DIR must be set to LLVM's root directory.")
endif()

get_cmake_property(VAR LLVM_BUILD_DIR)
if(NOT LLVM_BUILD_DIR)
    message(SEND_ERROR "LLVM_BUILD_DIR must be set to LLVM's build directory.")
endif()

set(SRC_LIST

    include/galena/shader_transpiler.h
    src/shader_transpiler.cpp)

include_directories(include
                    ${LLVM_DIR}/tools/clang/include)

find_library(LIBCLANG liblibclang.dll.a PATHS ${LLVM_BUILD_DIR}/lib)
if(NOT LIBCLANG)
    message(SEND_ERROR "Cannot find libclang library file.")
endif()

add_library(${PROJECT_NAME} SHARED ${SRC_LIST})
target_link_libraries(${PROJECT_NAME}
                      ${LIBCLANG}
                      Boost::program_options
                      Boost::filesystem
                      Dbghelp)
