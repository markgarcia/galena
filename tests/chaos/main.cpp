#include "galena/window_render_surface.h"
#include "galena/renderer.h"
#include "galena/window.h"
#include "galena/application.h"


int main() {
    galena::application application;
    galena::window window(640, 480, "galena chaos test application");

    galena::renderer renderer(galena::renderer::renderer_type::dx11);
    galena::window_render_surface window_render_surface(window, renderer);
    renderer.render_on(window_render_surface);

    while(application.is_open()) {
        application.process_events();
        window_render_surface.clear();
        window_render_surface.present();
    }
}
