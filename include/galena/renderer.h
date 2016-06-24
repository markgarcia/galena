#pragma once

#include "galena/window.h"
#include <memory>


namespace galena {


class window_render_surface;


namespace impl {


class renderer_impl {
public:
    virtual ~renderer_impl() = 0;

    virtual void render_on(window_render_surface& surface) = 0;
};


}


class renderer {
public:
    enum class renderer_type {
        dx11
    };

    const renderer_type type;

    renderer(renderer_type type);

    void render_on(window_render_surface& surface);

    impl::renderer_impl& get_impl() { return *m_impl; }

private:
    std::unique_ptr<impl::renderer_impl> m_impl;
};


}
