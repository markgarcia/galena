#include "galena/renderer/dx11/dx11_renderer.h"

#include "galena/renderer/dx11/dx11_window_render_surface.h"
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
    m_immediate_context->OMSetRenderTargets(1, &static_cast<dx11_window_render_surface&>(surface.get_impl()).get_render_target(), nullptr);
}


}
