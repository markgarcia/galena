#include "galena/renderer/dx11/dx11_renderer.h"

#include "galena/window/win32/window_impl.h"

#include <d3d11_1.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>


namespace galena {


dx11_renderer::dx11_renderer() {
    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* immediate_context = nullptr;

    if(FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0,
                                D3D11_SDK_VERSION, &device, nullptr, &immediate_context)))
    {
        throw std::runtime_error("Cannot create D3D11 device.");
    }

    m_device.reset(device);
    m_immediate_context.reset(immediate_context);
}

dx11_renderer::~dx11_renderer() = default;


}