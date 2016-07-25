project(galena-cpp-shader-compiler)
cmake_minimum_required(VERSION 2.8)

find_package(LLVM REQUIRED)
find_package(Clang REQUIRED)

set(SRC_LIST

    include/galena/shader_model.h
    include/galena/shader_compiler.h
    include/galena/source_location.h
    include/galena/shader_codegen/hlsl.h
    src/shader_model.cpp
    src/shader_compiler.cpp
    src/source_location.cpp
    src/shader_codegen/hlsl.cpp)

include_directories(include
                    ${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(LLVM_LIBS Symbolize)

add_library(${PROJECT_NAME} SHARED ${SRC_LIST})
target_link_libraries(${PROJECT_NAME}

                      ${LLVM_LIBS}
                      clangTooling

                      Boost::program_options
                      Boost::filesystem)

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ../../bin)
