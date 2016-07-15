#pragma once

#include <boost/variant.hpp>
#include <boost/type_index.hpp>
#include <string>
#include <vector>


namespace galena {
namespace shader_model {


class variable_base {
public:
    boost::typeindex::type_index get_type() const;
    const std::string& get_name();
};


class function_parameter : public variable_base {

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
    const std::string& get_name() const;

    const std::vector<function_parameter>& get_parameters() const;
    boost::typeindex::type_index get_return_type() const;
    const std::vector<operation>& get_operations() const;

private:
    std::string m_name;
};


}
}
