#include "galena/renderer/dx11/dx11_window_render_surface.h"

#include "galena/window/win32/window_impl.h"
#include <d3d11_1.h>


namespace galena {


dx11_window_render_surface::dx11_window_render_surface(const window& window, dx11_renderer& renderer)
    : m_device(renderer.get_device()) {
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


    ComPtr<IDXGIDevice> dxgi_device;
    m_device.As<IDXGIDevice>(&dxgi_device);

    ComPtr<IDXGIAdapter> adapter;
    dxgi_device->GetAdapter(&adapter);

    ComPtr<IDXGIFactory1> dxgi_factory;
    adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(dxgi_factory.GetAddressOf()));

    if(FAILED(dxgi_factory->CreateSwapChain(m_device.Get(), &swap_chain_desc, &m_swap_chain))) {
        throw new std::runtime_error("Cannot create swap chain.");
    }

    ComPtr<ID3D11Texture2D> back_buffer;
    if(FAILED(m_swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(back_buffer.GetAddressOf())))) {
        throw new std::runtime_error("Unable to get swap chain back buffer.");
    }

    if(FAILED(m_device->CreateRenderTargetView(back_buffer.Get(), nullptr, m_render_target.GetAddressOf()))) {
        throw new std::runtime_error("Unable to create swap chain back buffer render target.");
    }
}

dx11_window_render_surface::~dx11_window_render_surface() = default;

ComPtr<ID3D11RenderTargetView> dx11_window_render_surface::get_render_target() {
    return m_render_target;
}

void dx11_window_render_surface::clear() {
    ComPtr<ID3D11DeviceContext> immediate_context;
    m_device->GetImmediateContext(immediate_context.GetAddressOf());
    float clear_color[] = { 0, 0, 0, 1 };
    immediate_context->ClearRenderTargetView(m_render_target.Get(), clear_color);
}

void dx11_window_render_surface::present() {
    m_swap_chain->Present(0, 0);
}


}
