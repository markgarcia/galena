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

    template<typename return_type, typename... arg_types>
    void set_vertex_shader(return_type (*func)(arg_types...)) {
        compile_shader(reinterpret_cast<uint64_t>(func));
    }

    impl::renderer_impl& get_impl() { return *m_impl; }

private:
    std::unique_ptr<impl::renderer_impl> m_impl;

    std::string compile_shader(uint64_t func_address);
};


}
