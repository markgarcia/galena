#pragma once

#ifdef GALENA_SHADER_COMPILER

#ifndef GALENA_VERTEX_SHADER
#define GALENA_VERTEX_SHADER [[galena_vertex_shader]]
#endif

#ifndef GALENA_PIXEL_SHADER
#define GALENA_PIXEL_SHADER [[galena_pixel_shader]]
#endif

#else

#define GALENA_VERTEX_SHADER
#define GALENA_PIXEL_SHADER

#endif


namespace galena {

    struct float4 {
        float x, y, z, w;
    };

}
