#pragma once

#include "galena/window.h"
#include "galena/shader_compiler.h"

#include <memory>


namespace galena {


class window_render_surface;


namespace impl {


class compiled_vertex_shader {
public:
    virtual ~compiled_vertex_shader() = 0;
};


class renderer_impl {
public:
    virtual ~renderer_impl() = 0;

    virtual void render_on(window_render_surface& surface) = 0;
    virtual std::unique_ptr<compiled_vertex_shader> compile_vertex_shader(const shader_model::function&) = 0;
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

    template<typename return_type, typename... param_types>
    void set_vertex_shader(return_type (*func)(param_types...)) {
        compile_shader(reinterpret_cast<uint64_t>(func));
    }

    impl::renderer_impl& get_impl() { return *m_impl; }

private:
    std::unique_ptr<impl::renderer_impl> m_impl;
    shader_compiler m_compiler;

    std::unique_ptr<impl::compiled_vertex_shader> compile_shader(uint64_t func_address);
};


}
