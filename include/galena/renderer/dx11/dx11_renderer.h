#pragma once

#include "galena/renderer.h"
#include "galena/renderer/dx11/impl/com_ptr.h"


class ID3D11Device;
class ID3D11DeviceContext;


namespace galena {


class dx11_renderer : public renderer {
public:
    dx11_renderer();
    ~dx11_renderer();

private:
    impl::com_ptr<ID3D11Device> m_device;
    impl::com_ptr<ID3D11DeviceContext> m_immediate_context;
};


}