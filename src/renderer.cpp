#include "galena/renderer.h"

#include "galena/renderer/dx11/dx11_renderer.h"
#include "galena/source_location.h"


namespace galena {


impl::compiled_vertex_shader::~compiled_vertex_shader() = default;
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


void renderer::set_vertex_shader(uint64_t func_address) {
    auto location = locate_function(func_address);
    auto galena_include_dir = boost::filesystem::path(__FILE__).parent_path().parent_path() / "include";
    auto shader_model = m_compiler.compile(location.function_signature, location.source_file, galena_include_dir);

    auto shader = m_impl->compile_vertex_shader(shader_model);
    m_impl->set_vertex_shader(shader.get());
}


}
