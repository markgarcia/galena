#include <galena/shader/shader.h>
#include <galena/window_render_surface.h>
#include <galena/renderer.h>
#include <galena/window.h>
#include <galena/application.h>

#include <array>


namespace shaders {

galena::pixel_shader_position vertex_shader(galena::float4 pos);
galena::float4 pixel_shader(galena::pixel_shader_position);

}


int main() {
    galena::application application;
    galena::window window(640, 480, "galena chaos test application");

    galena::renderer renderer(galena::renderer::renderer_type::dx11);
    galena::window_render_surface window_render_surface(window, renderer);
    renderer.render_on(window_render_surface);

    auto vs_state = renderer.set_vertex_shader(shaders::vertex_shader);
    vs_state.set_input<0>(std::array<galena::float4, 3> {
        galena::float4 { 0.0f, 0.5f, 0.5f, 0.0f },
        galena::float4 { 0.5f, -0.5f, 0.5f, 0.0f },
        galena::float4 { -0.5f, -0.5f, 0.5f, 0.0f }
    });

    renderer.set_pixel_shader(shaders::pixel_shader);

    while(application.is_open()) {
        application.process_events();
        window_render_surface.clear();
        window_render_surface.present();
    }
}
