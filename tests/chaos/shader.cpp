#include <galena/shader/shader.h>

namespace shaders {


galena::float4 vertex_shader(galena::float4 pos) {
    return pos;
}


galena::float4 pixel_shader(galena::float4) {
    return { 1.0f, 1.0f, 0.0f, 1.0f };
}


}
