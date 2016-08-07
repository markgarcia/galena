#include "galena/renderer/dx11/dx11_renderer.h"

#include "galena/renderer/dx11/dx11_window_render_surface.h"
#include "galena/shader_codegen/hlsl.h"
#include "galena/shader/shader.h"

#include <d3dcompiler.h>
#include <d3d11_1.h>


namespace galena {


dx11_renderer::dx11_renderer() {
    D3D_FEATURE_LEVEL feature_levels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

    if(FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_DEBUG, feature_levels, 4,
                                D3D11_SDK_VERSION, &m_device, nullptr, &m_immediate_context)))
    {
        throw std::runtime_error("Cannot create D3D11 device.");
    }
}


dx11_renderer::~dx11_renderer() = default;


ComPtr<ID3D11Device> dx11_renderer::get_device() {
    return m_device;
}


void dx11_renderer::render_on(window_render_surface& surface) {
    auto render_target = static_cast<dx11_window_render_surface&>(surface.get_impl()).get_render_target();
    m_immediate_context->OMSetRenderTargets(1, render_target.GetAddressOf(), nullptr);

    D3D11_VIEWPORT viewport;
    viewport.Width = static_cast<float>(surface.get_width());
    viewport.Height = static_cast<float>(surface.get_height());
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    m_immediate_context->RSSetViewports(1, &viewport);
}


using shader_type = shader_codegen::hlsl_builder::shader_type;


void compile_shader(shader_type type, const shader_model::function& function,
                    ComPtr<ID3DBlob>& shader_blob) {
    std::stringstream hlsl;
    shader_codegen::hlsl_builder().build_function(function, type, hlsl);
    auto hlsl_code_str = hlsl.str();

    ComPtr<ID3DBlob> errors;
    if(FAILED(D3DCompile(hlsl_code_str.c_str(), hlsl_code_str.size(), (function.get_name() + ".hlsl").c_str(),
              nullptr, nullptr, function.get_name().c_str(),
              type == shader_type::vertex ? "vs_5_0"
                : type == shader_type::pixel ? "ps_5_0"
                : "",
              D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_WARNINGS_ARE_ERRORS,
              0,
              &shader_blob, &errors))) {
        throw std::runtime_error("Failed to compile shader.");
    }
}


class dx11_compiled_vertex_shader : public impl::compiled_vertex_shader {
public:
    ~dx11_compiled_vertex_shader() {}

    ComPtr<ID3D11VertexShader> shader;
    ComPtr<ID3D11InputLayout> input_layout;
};


std::unique_ptr<impl::compiled_vertex_shader> dx11_renderer::compile_vertex_shader(const shader_model::function& function) {
    ComPtr<ID3DBlob> shader_blob;
    compile_shader(shader_type::vertex, function, shader_blob);

    auto compiled_shader = std::make_unique<dx11_compiled_vertex_shader>();

    if(FAILED(m_device->CreateVertexShader(shader_blob->GetBufferPointer(), shader_blob->GetBufferSize(),
                                           nullptr, &compiled_shader->shader))) {
        throw std::runtime_error("Failed to create device shader.");
    }

    std::vector<D3D11_INPUT_ELEMENT_DESC> input_layout_desc;
    input_layout_desc.reserve(function.get_parameters().size());

    for(const auto& parameter : function.get_parameters()) {
        DXGI_FORMAT format;
        if(parameter.get_type() == boost::typeindex::type_id<galena::float4>()) {
            format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        }
        else {
            throw std::runtime_error("Unsupported type.");
        }

        D3D11_INPUT_ELEMENT_DESC input_desc;
        input_desc.SemanticName = shader_codegen::hlsl_builder::get_shader_type_semantic(parameter.get_type());
        input_desc.SemanticIndex = static_cast<UINT>(input_layout_desc.size());
        input_desc.Format = format;
        input_desc.InputSlot = static_cast<UINT>(input_layout_desc.size());
        input_desc.AlignedByteOffset = 0;
        input_desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        input_desc.InstanceDataStepRate = 0;

        input_layout_desc.emplace_back(input_desc);
    }

    if(FAILED(m_device->CreateInputLayout(input_layout_desc.data(), input_layout_desc.size(),
                                          shader_blob->GetBufferPointer(),
                                          shader_blob->GetBufferSize(),
                                          &compiled_shader->input_layout))) {
        throw std::runtime_error("Failed to create input layout.");
    }

    return std::move(compiled_shader);
}


class dx11_compiled_pixel_shader : public impl::compiled_pixel_shader {
public:
    ~dx11_compiled_pixel_shader() {}

    ComPtr<ID3D11PixelShader> shader;
};



std::unique_ptr<impl::compiled_pixel_shader> dx11_renderer::compile_pixel_shader(const shader_model::function& function) {
    ComPtr<ID3DBlob> shader_blob;
    compile_shader(shader_type::pixel, function, shader_blob);

    auto compiled_shader = std::make_unique<dx11_compiled_pixel_shader>();
    if(FAILED(m_device->CreatePixelShader(shader_blob->GetBufferPointer(), shader_blob->GetBufferSize(),
                                          nullptr, &compiled_shader->shader))) {
        throw std::runtime_error("Failed to create device shader.");
    }

    return std::move(compiled_shader);
}


void dx11_renderer::set_pixel_shader(impl::compiled_pixel_shader* shader) {
    m_immediate_context->PSSetShader(static_cast<dx11_compiled_pixel_shader*>(shader)->shader.Get(), nullptr, 0);
}


void dx11_renderer::set_vertex_shader(impl::compiled_vertex_shader* shader_) {
    auto shader = static_cast<dx11_compiled_vertex_shader*>(shader_);

    m_immediate_context->IASetInputLayout(shader->input_layout.Get());
    m_immediate_context->VSSetShader(shader->shader.Get(), nullptr, 0);
}


void dx11_renderer::set_vertex_shader_state(std::vector<impl::input_buffer> input_buffers) {
    std::vector<ComPtr<ID3D11Buffer>> buffers_storage;
    std::vector<ID3D11Buffer*> buffers_input;
    std::vector<UINT> strides;
    std::vector<UINT> offsets;

    for(const auto& input_buffer : input_buffers) {
        D3D11_BUFFER_DESC buffer_desc = {};
        buffer_desc.ByteWidth = input_buffer.buffer_size;
        buffer_desc.Usage = D3D11_USAGE_DEFAULT;
        buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        buffer_desc.CPUAccessFlags = 0;
        buffer_desc.MiscFlags = 0;
        buffer_desc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA subresource_data = {};
        subresource_data.pSysMem = input_buffer.data;
        subresource_data.SysMemPitch = 0;
        subresource_data.SysMemSlicePitch = 0;

        ComPtr<ID3D11Buffer> buffer;
        if(FAILED(m_device->CreateBuffer(&buffer_desc, &subresource_data, &buffer))) {
            throw std::runtime_error("Failed to create vertex buffer.");
        }

        buffers_input.emplace_back(buffer.Get());
        buffers_storage.emplace_back(std::move(buffer));
        strides.emplace_back(input_buffer.element_size);
        offsets.emplace_back(0);
    }

    m_immediate_context->IASetVertexBuffers(0, buffers_input.size(), buffers_input.data(),
                                            strides.data(), offsets.data());

    m_immediate_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}


void dx11_renderer::draw(unsigned int num_vertices) {
    m_immediate_context->Draw(num_vertices, 0);
}


}
