#pragma once

#include "galena/window_render_surface.h"
#include "galena/renderer.h"
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;


class ID3D11Device;
class ID3D11DeviceContext;


namespace galena {


class dx11_renderer : public renderer {
public:
    dx11_renderer();
    ~dx11_renderer();

    ComPtr<ID3D11Device> get_device();

    void render_on(window_render_surface& surface) override;

private:
    ComPtr<ID3D11Device> m_device;
    ComPtr<ID3D11DeviceContext> m_immediate_context;
};


}
