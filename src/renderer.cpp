#include "galena/renderer.h"

#include "galena/renderer/dx11/dx11_renderer.h"


namespace galena {


impl::renderer_impl::~renderer_impl() = default;


renderer::renderer(renderer_type type)
    : type(type) {
    switch(type) {
        case renderer_type::dx11:
            m_impl = std::make_unique<dx11_renderer>();
            break;

        default:
            throw std::invalid_argument("Invalid renderer type.");
    }
}

void renderer::render_on(window_render_surface& surface) {
    m_impl->render_on(surface);
}


}
