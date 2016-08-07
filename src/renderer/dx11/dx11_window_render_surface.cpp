#include "galena/renderer/dx11/dx11_window_render_surface.h"

#include "galena/window/win32/window_impl.h"

#include <d3d11_1.h>


namespace galena {


dx11_window_render_surface::dx11_window_render_surface(const window& window, dx11_renderer& renderer)
    : m_device(renderer.get_device()), m_width(window.get_width()), m_height(window.get_height()) {
    auto& window_impl = window.impl();

    ComPtr<IDXGIDevice> dxgi_device;
    if(FAILED(m_device.As<IDXGIDevice>(&dxgi_device))) {
        throw std::runtime_error("Failed to create DXGI device.");
    }

    ComPtr<IDXGIAdapter> adapter;
    if(FAILED(dxgi_device->GetAdapter(&adapter))) {
        throw std::runtime_error("Failed to obtain DXGI adapter.");
    }

    ComPtr<IDXGIFactory1> dxgi_factory;
    if(FAILED(adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(dxgi_factory.GetAddressOf())))) {
        throw std::runtime_error("Failed to obtain DXGI factory.");
    }

    ComPtr<IDXGIFactory2> dxgi_factory2;
    if(SUCCEEDED(dxgi_factory.As<IDXGIFactory2>(&dxgi_factory2))) {
        DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {};
        swap_chain_desc.Width = m_width;
        swap_chain_desc.Height = m_height;
        swap_chain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swap_chain_desc.SampleDesc.Count = 1;
        swap_chain_desc.SampleDesc.Quality = 0;
        swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swap_chain_desc.BufferCount = 1;

        ComPtr<IDXGISwapChain1> swap_chain1;
        if(FAILED(dxgi_factory2->CreateSwapChainForHwnd(m_device.Get(), window_impl.native_handle(),
                                                        &swap_chain_desc, nullptr, nullptr,
                                                        &swap_chain1))) {
            throw std::runtime_error("Failed to create swap chain.");
        }

        swap_chain1.As(&m_swap_chain);
    }
    else {
        DXGI_SWAP_CHAIN_DESC swap_chain_desc = {};
        swap_chain_desc.BufferCount = 1;
        swap_chain_desc.BufferDesc.Width = m_width;
        swap_chain_desc.BufferDesc.Height = m_height;
        swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swap_chain_desc.BufferDesc.RefreshRate.Numerator = 60;
        swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
        swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swap_chain_desc.OutputWindow = window_impl.native_handle();
        swap_chain_desc.SampleDesc.Count = 1;
        swap_chain_desc.SampleDesc.Quality = 0;
        swap_chain_desc.Windowed = TRUE;

        if(FAILED(dxgi_factory->CreateSwapChain(m_device.Get(), &swap_chain_desc, &m_swap_chain))) {
            throw new std::runtime_error("Cannot create swap chain.");
        }
    }

    dxgi_factory->MakeWindowAssociation(window_impl.native_handle(), DXGI_MWA_NO_ALT_ENTER);

    ComPtr<ID3D11Texture2D> back_buffer;
    if(FAILED(m_swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(back_buffer.GetAddressOf())))) {
        throw std::runtime_error("Unable to get swap chain back buffer.");
    }

    if(FAILED(m_device->CreateRenderTargetView(back_buffer.Get(), nullptr, &m_render_target))) {
        throw std::runtime_error("Unable to create swap chain back buffer render target.");
    }
}


dx11_window_render_surface::~dx11_window_render_surface() = default;


ComPtr<ID3D11RenderTargetView> dx11_window_render_surface::get_render_target() {
    return m_render_target;
}


void dx11_window_render_surface::clear() {
    ComPtr<ID3D11DeviceContext> immediate_context;
    m_device->GetImmediateContext(&immediate_context);
    float clear_color[] = { 0, 0, 0, 1 };
    immediate_context->ClearRenderTargetView(m_render_target.Get(), clear_color);
}


void dx11_window_render_surface::present() {
    m_swap_chain->Present(0, 0);
}


unsigned int dx11_window_render_surface::get_width() const {
    return m_width;
}


unsigned int dx11_window_render_surface::get_height() const {
    return m_height;
}


}
