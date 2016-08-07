#pragma once

#include "galena/renderer/dx11/dx11_renderer.h"
#include "galena/window_render_surface.h"


class ID3D11Device;
class IDXGISwapChain;
class ID3D11RenderTargetView;


namespace galena {


class dx11_window_render_surface : public impl::window_render_surface_impl {
public:
    dx11_window_render_surface(const window& window, dx11_renderer& renderer);
    ~dx11_window_render_surface();

    void clear() override;
    void present() override;

    unsigned int get_width() const override;
    unsigned int get_height() const override;

    ComPtr<ID3D11RenderTargetView> get_render_target();

private:
    ComPtr<ID3D11Device> m_device;
    ComPtr<IDXGISwapChain> m_swap_chain;
    ComPtr<ID3D11RenderTargetView> m_render_target;

    unsigned int m_width = 0;
    unsigned int m_height = 0;
};


}
