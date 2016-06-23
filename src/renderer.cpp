#include "galena/renderer.h"

#include "galena/renderer/dx11/dx11_renderer.h"


namespace galena {


renderer::~renderer() {}


std::unique_ptr<renderer> renderer::create(renderer_type type) {
    switch(type) {
        case renderer_type::dx11:
            return std::make_unique<dx11_renderer>();

        default:
            return nullptr;
    }
}


}