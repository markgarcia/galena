#pragma once

#include "galena/shader/shader_model.h"

#include <boost/type_index.hpp>
#include <sstream>


namespace galena {
namespace shader_codegen {


class hlsl_builder {
public:
    void build_function(const shader_model::function& function, std::stringstream& hlsl);
    void build_parameter_list(const std::vector<shader_model::function_parameter>& parameters,
                              std::stringstream& hlsl);
    void build_function_body(const std::vector<shader_model::operation>& operations,
                             std::stringstream& hlsl);
    void build_return_operation(const shader_model::return_operation& return_operation,
                                std::stringstream& hlsl);
    void build_expression(const shader_model::expression& expression,
                          std::stringstream& hlsl);

private:
    const char* get_shader_type_hlsl_equivalent(boost::typeindex::type_index type);
};


}
}
