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

    include/galena/shader_compiler.h
    include/galena/source_location.h
    src/shader_compiler.cpp
    src/source_location.cpp)

include_directories(include
                    ${LLVM_DIR}/include
                    ${LLVM_DIR}/tools/clang/include
                    ${LLVM_BUILD_DIR}/include)

find_library(LIB_CLANG liblibclang.dll.a PATHS ${LLVM_BUILD_DIR}/lib)
if(NOT LIB_CLANG)
    message(SEND_ERROR "Cannot find libclang library file.")
endif()

find_library(LIB_LLVM_SYMBOLIZE LLVMSymbolize PATHS ${LLVM_BUILD_DIR}/lib)
find_library(LIB_LLVM_SUPPORT LLVMSupport PATHS ${LLVM_BUILD_DIR}/lib)
find_library(LIB_LLVM_OBJECT LLVMObject PATHS ${LLVM_BUILD_DIR}/lib)
find_library(LIB_LLVM_DEBUG_INFO_DWARF LLVMDebugInfoDWARF PATHS ${LLVM_BUILD_DIR}/lib)
find_library(LIB_LLVM_DEBUG_INFO_PDB LLVMDebugInfoPDB PATHS ${LLVM_BUILD_DIR}/lib)
find_library(LIB_LLVM_MC LLVMMC PATHS ${LLVM_BUILD_DIR}/lib)
find_library(LIB_LLVM_CORE LLVMCore PATHS ${LLVM_BUILD_DIR}/lib)
find_library(LIB_LLVM_MC_PARSER LLVMMCParser PATHS ${LLVM_BUILD_DIR}/lib)
find_library(LIB_LLVM_BIT_READER LLVMBitReader PATHS ${LLVM_BUILD_DIR}/lib)

if(NOT LIB_LLVM_SYMBOLIZE
    OR NOT LIB_LLVM_SUPPORT
    OR NOT LIB_LLVM_OBJECT
    OR NOT LIB_LLVM_DEBUG_INFO_DWARF
    OR NOT LIB_LLVM_DEBUG_INFO_PDB
    OR NOT LIB_LLVM_MC
    OR NOT LIB_LLVM_CORE
    OR NOT LIB_LLVM_MC_PARSER
    OR NOT LIB_LLVM_BIT_READER)
    message(SEND_ERROR "Cannot find LLVM library file(s).")
endif()

add_library(${PROJECT_NAME} SHARED ${SRC_LIST})
target_link_libraries(${PROJECT_NAME}
                      ${LIB_CLANG}
                      ${LIB_LLVM_DEBUG_INFO_DWARF}
                          ${LIB_LLVM_DEBUG_INFO_PDB}
                          ${LIB_LLVM_OBJECT}
                          ${LIB_LLVM_SUPPORT}
                          ${LIB_LLVM_SYMBOLIZE}
                          ${LIB_LLVM_DEBUG_INFO_DWARF}
                          ${LIB_LLVM_DEBUG_INFO_PDB}
                          ${LIB_LLVM_OBJECT}
                          ${LIB_LLVM_BIT_READER}
                          ${LIB_LLVM_CORE}
                          ${LIB_LLVM_MC_PARSER}
                          ${LIB_LLVM_MC}
                          ${LIB_LLVM_SUPPORT}
                      Boost::program_options
                      Boost::filesystem
                      Dbghelp)

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ../bin)
