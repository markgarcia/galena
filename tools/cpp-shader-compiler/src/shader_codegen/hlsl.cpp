#include "galena/shader_codegen/hlsl.h"

#include "galena/shader/shader.h"


namespace galena {
namespace shader_codegen {


const char* hlsl_builder::get_shader_type_hlsl_equivalent(boost::typeindex::type_index type) {
    if(type == boost::typeindex::type_id<galena::float4>()
        || type == boost::typeindex::type_id<galena::pixel_shader_position>()) {
        return "float4";
    }

    throw std::runtime_error("Unknown/Unsupported type.");
}


const char* hlsl_builder::get_shader_type_semantic(boost::typeindex::type_index type,
                                                   boost::optional<bool&> is_system_value) {
    if(type == boost::typeindex::type_id<galena::float4>()) {
        return "POSITION";
    }
    else if(type == boost::typeindex::type_id<galena::pixel_shader_position>()) {
        if(is_system_value) is_system_value.get() = true;
        return "SV_POSITION";
    }

    throw std::runtime_error("Unknown/Unsupported type.");
}


void hlsl_builder::build_function(const shader_model::function& function, shader_type type, std::stringstream& hlsl) {
    hlsl << get_shader_type_hlsl_equivalent(function.get_return_type())
         << ' '
         << function.get_name();

    hlsl << '(';
    build_parameter_list(function.get_parameters(), hlsl);
    hlsl << ')';

    if(type == shader_type::pixel && function.get_return_type() == boost::typeindex::type_id<galena::float4>()) {
        hlsl << ":SV_TARGET";
    }
    else {
        hlsl << ':' << get_shader_type_semantic(function.get_return_type());
    }

    hlsl << '{';
    build_function_body(function.get_operations(), hlsl);
    hlsl << '}';
}


void hlsl_builder::build_parameter_list(const std::vector<shader_model::function_parameter>& parameters,
                                        std::stringstream& hlsl) {
    if(parameters.empty()) return;

    for(std::size_t i = 0; i < parameters.size(); ++i) {
        hlsl << get_shader_type_hlsl_equivalent(parameters[i].get_type())
             << ' '
             << parameters[i].get_name()
             << ':';

        bool is_system_value;
        hlsl << get_shader_type_semantic(parameters[i].get_type(), is_system_value);
        if(!is_system_value) hlsl << i;

        if(i < parameters.size() - 1) hlsl << ',';
    }
}


void hlsl_builder::build_function_body(const std::vector<shader_model::operation>& operations,
                                       std::stringstream& hlsl) {
    class operation_visitor : boost::static_visitor<shader_model::operation> {
    public:
        operation_visitor(hlsl_builder& builder, std::stringstream& hlsl)
            : builder(builder), hlsl(hlsl) {}

        void operator()(const shader_model::return_operation& return_operation) const {
            builder.build_return_operation(return_operation, hlsl);
        }

    private:
        hlsl_builder& builder;
        std::stringstream& hlsl;
    };

    for(const auto& operation : operations) {
        boost::apply_visitor(operation_visitor(*this, hlsl), operation);
    }
}


void hlsl_builder::build_return_operation(const shader_model::return_operation& return_operation,
                                          std::stringstream& hlsl) {
    hlsl << "return ";
    build_expression(return_operation.get_expression(), hlsl);
    hlsl << ';';
}


void hlsl_builder::build_expression(const shader_model::expression& expression,
                                    std::stringstream& hlsl) {
    class expression_visitor : boost::static_visitor<shader_model::expression> {
    public:
        expression_visitor(hlsl_builder& builder, std::stringstream& hlsl)
            : builder(builder), hlsl(hlsl) {}

        void operator()(const shader_model::literal_value_expression& literal_expr) const {
            auto type = literal_expr.get_type();
            if(type == boost::typeindex::type_id<float>()) {
                hlsl << std::showpoint << literal_expr.get_value<float>() << 'f';
            }
        }

        void operator()(const shader_model::variable_reference_expression& var_ref) const {
            if(auto function_parameter = boost::get<shader_model::function_parameter>(&var_ref.get_variable())) {
                hlsl << function_parameter->get_name();
            }
        }

        void operator()(const shader_model::variable_construct_expression& construct_expr) const {
            hlsl << get_shader_type_hlsl_equivalent(construct_expr.get_type())
                 << '(';

            const auto& arguments = construct_expr.get_arguments();
            for(std::size_t i = 0; i < arguments.size(); ++i) {
                builder.build_expression(arguments[i], hlsl);
                if(i < arguments.size() - 1) hlsl << ',';
            }

            hlsl << ')';
        }

    private:
        hlsl_builder& builder;
        std::stringstream& hlsl;
    };

    boost::apply_visitor(expression_visitor(*this, hlsl), expression);
}


}
}
