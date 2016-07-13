
add_executable(chaos
               chaos/main.cpp
               chaos/shader.cpp)
target_link_libraries(chaos galena galena-cpp-shader-compiler)

set_target_properties(chaos PROPERTIES RUNTIME_OUTPUT_DIRECTORY ../bin)
