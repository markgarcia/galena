#include <galena/shader/shader.h>


GALENA_VERTEX_SHADER galena::float4 vertex_shader(galena::float4 pos) {
    return pos;
}


GALENA_PIXEL_SHADER galena::float4 pixel_shader(galena::float4)
{
    return { 1.0f, 1.0f, 0.0f, 1.0f };
}
