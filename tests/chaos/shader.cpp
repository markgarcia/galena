#include <galena/shader/shader.h>

namespace shaders {


galena::pixel_shader_position vertex_shader(galena::float4 pos) {
    return pos;
}


galena::float4 pixel_shader(galena::pixel_shader_position) {
    return { 1.0f, 1.0f, 0.0f, 1.0f };
}


}
