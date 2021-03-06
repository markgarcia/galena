#pragma once

#include <boost/any.hpp>
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
    const std::string& get_name() const { return m_name; }

private:
    boost::typeindex::type_index m_type;
    std::string m_name;
};


class function_parameter : public variable_base {
public:
    using variable_base::variable_base;
};


using variable = boost::variant<function_parameter>;


class literal_value_expression;
class variable_reference_expression;
class variable_construct_expression;
using expression = boost::variant<literal_value_expression,
                                  variable_reference_expression,
                                  boost::recursive_wrapper<variable_construct_expression>>;


class literal_value_expression {
public:
    template<typename type>
    literal_value_expression(type&& value) {
        set_value(std::forward<type>(value));
    }

    boost::typeindex::type_index get_type() const { return m_type; }

    template<typename type>
    void set_value(type&& value) {
        m_type = boost::typeindex::type_id<type>();
        m_value = std::forward<type>(value);
    }

    template<typename type>
    type get_value() const {
        return boost::any_cast<type>(m_value);
    }

private:
    boost::any m_value;
    boost::typeindex::type_index m_type;
};


class variable_reference_expression {
public:
    variable_reference_expression(variable var) : m_var(std::move(var)) {}

    const variable& get_variable() const { return m_var; }

private:
    variable m_var;
};


class variable_construct_expression {
public:
    variable_construct_expression() = default;

    variable_construct_expression(boost::typeindex::type_index type)
        : m_type(type) {}

    void set_type(boost::typeindex::type_index type) { m_type = type; }
    boost::typeindex::type_index get_type() const { return m_type; }

    void add_argument(expression expr) { m_arguments.emplace_back(std::move(expr)); }
    const std::vector<expression>& get_arguments() const { return m_arguments; }

private:
    boost::typeindex::type_index m_type;
    std::vector<expression> m_arguments;
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

    void set_return_type(boost::typeindex::type_index type) { m_return_type = type; }
    boost::typeindex::type_index get_return_type() const { return m_return_type; }

    void add_operation(operation op) { m_operations.emplace_back(std::move(op)); }
    const std::vector<operation>& get_operations() const { return m_operations; }

private:
    std::string m_name;
    boost::typeindex::type_index m_return_type;
    std::vector<function_parameter> m_parameters;
    std::vector<operation> m_operations;
};


}
}
