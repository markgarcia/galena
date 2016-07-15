#pragma once

#include <boost/variant.hpp>
#include <boost/type_index.hpp>
#include <string>
#include <vector>


namespace galena {
namespace shader_model {


class variable_base {
public:
    variable_base() = default;
    variable_base(boost::typeindex::type_index type, std::string name)
        : m_type(type), m_name(std::move(name)) {}

    void set_type(boost::typeindex::type_index type) { m_type = type; }
    boost::typeindex::type_index get_type() const { return m_type; }

    void set_name(std::string name) { m_name = std::move(name); }
    const std::string& get_name() { return m_name; }

private:
    boost::typeindex::type_index m_type;
    std::string m_name;
};


class function_parameter : public variable_base {
public:
    using variable_base::variable_base;
};


using variable = boost::variant<function_parameter>;


class variable_reference_expression;
using expression = boost::variant<variable_reference_expression>;


class variable_reference_expression {
public:
    variable_reference_expression(variable var) : m_var(std::move(var)) {}

    const variable& get_variable() const { return m_var; }

private:
    variable m_var;
};



class return_operation {
public:
    return_operation(expression expr) : m_expr(std::move(expr)) {}

    const expression& get_expression() const { return m_expr; }

private:
    expression m_expr;
};


using operation = boost::variant<return_operation>;


class function {
public:
    void set_name(std::string name) { m_name = std::move(name); }
    const std::string& get_name() const { return m_name; }

    void add_parameter(function_parameter parameter) { m_parameters.emplace_back(std::move(parameter)); }
    const std::vector<function_parameter>& get_parameters() const { return m_parameters; }

    boost::typeindex::type_index get_return_type() const;
    const std::vector<operation>& get_operations() const;

private:
    std::string m_name;
    std::vector<function_parameter> m_parameters;
};


}
}
