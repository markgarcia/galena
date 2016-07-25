#pragma once

namespace galena {

    struct float4 {
        float x, y, z, w;
    };

    struct pixel_shader_position : float4 {
        pixel_shader_position(const float4& rhs) : float4(rhs) {}
        pixel_shader_position& operator=(const float4& rhs) {
            static_cast<float4&>(*this) = rhs;
            return *this;
        }
    };

}
