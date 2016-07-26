#pragma once

#include "galena/shader_model.h"

#include <boost/optional.hpp>
#include <boost/type_index.hpp>
#include <sstream>


namespace galena {
namespace shader_codegen {


class BOOST_SYMBOL_EXPORT hlsl_builder {
public:
    enum class shader_type {
        vertex, pixel
    };

    void build_function(const shader_model::function& function, shader_type type,
                        std::stringstream& hlsl);
    void build_parameter_list(const std::vector<shader_model::function_parameter>& parameters,
                              std::stringstream& hlsl);
    void build_function_body(const std::vector<shader_model::operation>& operations,
                             std::stringstream& hlsl);
    void build_return_operation(const shader_model::return_operation& return_operation,
                                std::stringstream& hlsl);
    void build_expression(const shader_model::expression& expression,
                          std::stringstream& hlsl);

    static const char* get_shader_type_hlsl_equivalent(boost::typeindex::type_index type);
    static const char* get_shader_type_semantic(boost::typeindex::type_index type,
                                                boost::optional<bool&> is_system_value = boost::none);
};


}
}
