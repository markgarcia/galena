#pragma once

#include "galena/window.h"
#include <memory>


namespace galena {


class window_render_surface;


class renderer {
public:
    enum class renderer_type {
        dx11
    };

    const renderer_type type;

    renderer(renderer_type type);
    virtual ~renderer() = 0;

    virtual void render_on(window_render_surface& surface) = 0;

    static std::unique_ptr<renderer> create(renderer_type type);
};


}
