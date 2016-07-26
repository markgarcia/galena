#pragma once

#include "galena/window.h"
#include "galena/shader_compiler.h"

#include <memory>
#include <tuple>


namespace galena {


class window_render_surface;


namespace impl {


class compiled_vertex_shader {
public:
    virtual ~compiled_vertex_shader() = 0;
};


class compiled_pixel_shader {
public:
    virtual ~compiled_pixel_shader() = 0;
};


struct input_buffer {
    const void* data;
    std::size_t buffer_size;
    std::size_t element_size;
};


class renderer_impl {
public:
    virtual ~renderer_impl() = 0;

    virtual void render_on(window_render_surface& surface) = 0;

    virtual std::unique_ptr<compiled_vertex_shader> compile_vertex_shader(const shader_model::function&) = 0;
    virtual void set_vertex_shader(compiled_vertex_shader* shader) = 0;
    virtual void set_vertex_shader_state(std::vector<input_buffer> input_buffers) = 0;

    virtual std::unique_ptr<compiled_pixel_shader> compile_pixel_shader(const shader_model::function&) = 0;
    virtual void set_pixel_shader(compiled_pixel_shader* shader) = 0;
};


}


template<typename... input_param_types>
class vertex_shader_state {
public:
    template<unsigned int param_index, template<typename type> class buffer_type>
    void set_input(const buffer_type<std::tuple_element_t<param_index, std::tuple<input_param_types...>>>& source_buffer) {
        auto buffer = std::get<param_index>(m_buffers);
        buffer.clear();
        buffer.resize(source_buffer.size());
        std::copy(source_buffer.begin(), source_buffer.end(), buffer.begin());
    }

    template<unsigned int param_index, std::size_t array_size>
    void set_input(const std::array<std::tuple_element_t<param_index, std::tuple<input_param_types...>>, array_size>& source_buffer) {
        auto buffer = std::get<param_index>(m_buffers);
        buffer.clear();
        buffer.resize(source_buffer.size());
        std::copy(source_buffer.begin(), source_buffer.end(), buffer.begin());
    }

    template<unsigned int param_index>
    const std::vector<std::tuple_element_t<param_index, std::tuple<input_param_types...>>>& get_input() const {
        return std::get<param_index>(m_buffers);
    }

private:
    std::tuple<std::vector<input_param_types>...> m_buffers;
};


class renderer {
public:
    enum class renderer_type {
        dx11
    };

    const renderer_type type;

    renderer(renderer_type type);

    void render_on(window_render_surface& surface);

    template<typename return_type, typename... param_types>
    vertex_shader_state<param_types...> set_vertex_shader(return_type (*func)(param_types...)) {
        set_vertex_shader(reinterpret_cast<uint64_t>(func));
        return {};
    }

    template<typename... input_param_types>
    void set_vertex_shader_state(const vertex_shader_state<input_param_types...>& state) {
        std::vector<impl::input_buffer> input_buffers;
        input_buffers.reserve(sizeof...(input_param_types));
        set_vertex_shader_state_helper<0, sizeof...(input_param_types)>::help(state, input_buffers);
        m_impl->set_vertex_shader_state(input_buffers);
    }

    template<typename return_type, typename... param_types>
    void set_pixel_shader(return_type (*func)(param_types...)) {
        set_pixel_shader(reinterpret_cast<uint64_t>(func));
    }

    impl::renderer_impl& get_impl() { return *m_impl; }

private:
    std::unique_ptr<impl::renderer_impl> m_impl;
    shader_compiler m_compiler;

    void set_vertex_shader(uint64_t func_address);
    void set_pixel_shader(uint64_t func_address);


    template<unsigned int param_index, unsigned int end>
    struct set_vertex_shader_state_helper {
        template<typename... input_param_types>
        static void help(const vertex_shader_state<input_param_types...>& state,
                         std::vector<impl::input_buffer>& input_buffers) {
            const auto& input = state.template get_input<param_index>();
            input_buffers.emplace_back(input.data(),
                                       sizeof(typename decltype(input)::value_type) * input.size(),
                                       sizeof(typename decltype(input)::value_type));
            set_vertex_shader_state_helper<param_index + 1, end>::help(state, input_buffers);
        }
    };

    template<unsigned int end>
    struct set_vertex_shader_state_helper<end, end> {
        template<typename... input_param_types>
        static void help(const vertex_shader_state<input_param_types...>&,
                         std::vector<impl::input_buffer>&) {}
    };
};


}
