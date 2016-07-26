#pragma once

#include "galena/window_render_surface.h"
#include "galena/renderer.h"
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;


class ID3D11Device;
class ID3D11DeviceContext;


namespace galena {


class dx11_renderer : public impl::renderer_impl {
public:
    dx11_renderer();
    ~dx11_renderer();

    ComPtr<ID3D11Device> get_device();

    void render_on(window_render_surface& surface) override;
    std::unique_ptr<impl::compiled_vertex_shader> compile_vertex_shader(const shader_model::function&) override;
    void set_vertex_shader(impl::compiled_vertex_shader* shader) override;
    void set_vertex_shader_state(std::vector<impl::input_buffer> input_buffers) override;

    std::unique_ptr<impl::compiled_pixel_shader> compile_pixel_shader(const shader_model::function&) override;
    void set_pixel_shader(impl::compiled_pixel_shader* shader) override;

private:
    ComPtr<ID3D11Device> m_device;
    ComPtr<ID3D11DeviceContext> m_immediate_context;
};


}
