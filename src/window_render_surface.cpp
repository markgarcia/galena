#include "galena/window_render_surface.h"

#include "galena/renderer/dx11/dx11_window_render_surface.h"
#include "galena/renderer/dx11/dx11_renderer.h"


namespace galena {


impl::window_render_surface_impl::~window_render_surface_impl() = default;

window_render_surface::window_render_surface(const window& window, renderer& renderer) {
    if(renderer.type == renderer::renderer_type::dx11) {
        m_impl = std::make_unique<dx11_window_render_surface>(window, static_cast<dx11_renderer&>(renderer));
    }
}

void window_render_surface::clear() {
    m_impl->clear();
}

void window_render_surface::present() {
    m_impl->present();
}


}
