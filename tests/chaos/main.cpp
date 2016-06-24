#include "galena/renderer.h"
#include "galena/window.h"
#include "galena/application.h"

using namespace galena;


int main() {
    application application;
    window window(640, 480, "galena chaos test application");

    auto renderer = renderer::create(renderer::renderer_type::dx11);
    renderer->render_on(window);

    while(application.is_open()) {
        application.process_events();
    }
}