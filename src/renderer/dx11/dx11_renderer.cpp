#include "galena/renderer/dx11/dx11_renderer.h"

#include "galena/window/win32/window_impl.h"

#include <d3d11_1.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif


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


void dx11_renderer::render_on(const window& window) {
    auto hwnd = window.impl()->native_handle();

    RECT window_client_rect;
    GetClientRect(hwnd, &window_client_rect);

    DXGI_SWAP_CHAIN_DESC swap_chain_desc = {};
    swap_chain_desc.BufferCount = 1;
    swap_chain_desc.BufferDesc.Width = static_cast<UINT>(window_client_rect.right);
    swap_chain_desc.BufferDesc.Height = static_cast<UINT>(window_client_rect.bottom);
    swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap_chain_desc.BufferDesc.RefreshRate.Numerator = 60;
    swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
    swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc.OutputWindow = hwnd;
    swap_chain_desc.SampleDesc.Count = 1;
    swap_chain_desc.SampleDesc.Quality = 0;
    swap_chain_desc.Windowed = TRUE;


    IDXGIDevice* dxgi_device = nullptr;
    m_device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgi_device));

    IDXGIAdapter* adapter = nullptr;
    dxgi_device->GetAdapter(&adapter);

    IDXGIFactory1* dxgi_factory = nullptr;
    adapter->GetParent( __uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgi_factory) );
    adapter->Release();

    dxgi_device->Release();

    IDXGISwapChain* swap_chain = nullptr;
    if(FAILED(dxgi_factory->CreateSwapChain(m_device.get(), &swap_chain_desc, &swap_chain))) {
        throw new std::runtime_error("Cannot create swap chain.");
    }

    dxgi_factory->Release();

    m_swap_chain.reset(swap_chain);
}


}