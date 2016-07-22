#include "galena/renderer/dx11/dx11_renderer.h"

#include "galena/renderer/dx11/dx11_window_render_surface.h"
#include "galena/shader/codegen/hlsl.h"

#include <d3dcompiler.h>
#include <d3d11_1.h>


namespace galena {


dx11_renderer::dx11_renderer() {
    if(FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0,
                                D3D11_SDK_VERSION, &m_device, nullptr, &m_immediate_context)))
    {
        throw std::runtime_error("Cannot create D3D11 device.");
    }
}


dx11_renderer::~dx11_renderer() = default;


ComPtr<ID3D11Device> dx11_renderer::get_device() {
    return m_device;
}


void dx11_renderer::render_on(window_render_surface& surface) {
    m_immediate_context->OMSetRenderTargets(1,
                                            &static_cast<dx11_window_render_surface&>(surface.get_impl())
                                                .get_render_target(),
                                            nullptr);
}


class dx11_compiled_shader : public impl::compiled_vertex_shader {
public:
    ~dx11_compiled_shader() {}

    ComPtr<ID3DBlob> shader_blob;
};


std::unique_ptr<impl::compiled_vertex_shader> dx11_renderer::compile_vertex_shader(const shader_model::function& function) {
    std::stringstream hlsl;
    shader_codegen::hlsl_builder().build_function(function, hlsl);
    auto hlsl_code_str = hlsl.str();

    ComPtr<ID3DBlob> shader_blob;
    ComPtr<ID3DBlob> errors;
    if(FAILED(D3DCompile(hlsl_code_str.c_str(), hlsl_code_str.size(), (function.get_name() + ".hlsl").c_str(),
              nullptr, nullptr, function.get_name().c_str(), "vs_5_0",
              D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL0 | D3DCOMPILE_WARNINGS_ARE_ERRORS,
              0,
              &shader_blob, &errors))) {
        throw std::runtime_error("Failed to compile shader.");
    }

    auto compiled_shader = std::make_unique<dx11_compiled_shader>();
    compiled_shader->shader_blob = std::move(shader_blob);
    return std::move(compiled_shader);
}


}
