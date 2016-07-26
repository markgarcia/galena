#include "galena/shader_compiler.h"

#include "galena/shader/shader.h"

#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Tooling/Tooling.h>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/optional.hpp>
#include <boost/operators.hpp>
#include <iostream>
#include <algorithm>


namespace galena {


class clang_ast_consumer : public clang::ASTConsumer {
    class match_finder_wrapper {
    public:
        match_finder_wrapper(clang::ASTContext& context) : m_context(context) {}

        template<typename node_type, typename matcher_type, typename callback_type>
        void add_matcher(const matcher_type& matcher, std::string id,
                         callback_type&& callback) {
            class match_callback_wrapper : public clang::ast_matchers::MatchFinder::MatchCallback {
            public:
                match_callback_wrapper(std::string id, callback_type&& callback)
                    : m_id(std::move(id)), m_callback(std::forward<callback_type>(callback)) {}

                void run(const clang::ast_matchers::MatchFinder::MatchResult& result) override {
                    m_callback(result.Nodes.getNodeAs<node_type>(m_id));
                }

            private:
                std::string m_id;
                typename std::remove_reference<callback_type>::type m_callback;
            };

            auto match_callback = std::make_unique<match_callback_wrapper>(std::move(id), std::forward<callback_type>(callback));
            m_match_finder.addMatcher(matcher, match_callback.get());
            m_match_callbacks.emplace_back(std::move(match_callback));
        }

        template<typename type>
        struct match_id {
            match_id(std::string id) : id(std::move(id)) {}
            std::string id;
        };

        template<typename matcher_type, typename... node_types, typename callback_type>
        void add_multiple_matchers(const matcher_type& matcher,
                                   std::tuple<match_id<node_types>...> ids,
                                   callback_type&& callback) {
            class match_callback_wrapper : public clang::ast_matchers::MatchFinder::MatchCallback {
            public:
                match_callback_wrapper(std::tuple<match_id<node_types>...> ids, callback_type&& callback)
                    : m_ids(std::move(ids)), m_callback(std::forward<callback_type>(callback)) {}

                void run(const clang::ast_matchers::MatchFinder::MatchResult& result) override {
                    std::tuple<std::add_pointer_t<std::add_const_t<node_types>>...> arguments;
                    callback_arguments_helper<0, sizeof...(node_types), node_types...>::help(result, m_ids, arguments);
                    call_callback(m_callback, arguments, std::make_index_sequence<sizeof...(node_types)>());
                }

            private:
                std::tuple<match_id<node_types>...> m_ids;
                typename std::remove_reference<callback_type>::type m_callback;
            };

            auto match_callback = std::make_unique<match_callback_wrapper>(std::move(ids), std::forward<callback_type>(callback));
            m_match_finder.addMatcher(matcher, match_callback.get());
            m_match_callbacks.emplace_back(std::move(match_callback));
        }

        void match() {
            m_match_finder.matchAST(m_context);
        }

    private:
        template<typename callback_type, typename... node_types, std::size_t... idx>
        static void call_callback(callback_type&& callback, std::tuple<node_types...> arguments, std::index_sequence<idx...>) {
            std::forward<callback_type>(callback)(std::get<idx>(arguments)...);
        }

        template<unsigned int idx, unsigned int end, typename... node_types>
        struct callback_arguments_helper {
            static void help(const clang::ast_matchers::MatchFinder::MatchResult& result,
                             const std::tuple<match_id<node_types>...>& ids,
                             std::tuple<std::add_pointer_t<std::add_const_t<node_types>>...>& arguments) {
                using node_type = std::tuple_element_t<idx, std::tuple<node_types...>>;
                std::get<idx>(arguments)
                    = result.Nodes.template getNodeAs<node_type>(
                            std::get<idx>(ids).id);
                callback_arguments_helper<idx + 1, end, node_types...>::help(result, ids, arguments);
            }
        };

        template<unsigned int end, typename... node_types>
        struct callback_arguments_helper<end, end, node_types...> {
            static void help(const clang::ast_matchers::MatchFinder::MatchResult&,
                             std::tuple<match_id<node_types>...>,
                             std::tuple<std::add_pointer_t<std::add_const_t<node_types>>...>&) {}
        };


        clang::ASTContext& m_context;
        clang::ast_matchers::MatchFinder m_match_finder;
        std::vector<std::unique_ptr<clang::ast_matchers::MatchFinder::MatchCallback>> m_match_callbacks;
    };

public:
    clang_ast_consumer(std::string function_signature, shader_model::function& function)
        : m_function_signature(std::move(function_signature)), m_function(function) {}

    void HandleTranslationUnit(clang::ASTContext& context) override {
        using namespace clang::ast_matchers;

        match_finder_wrapper galena_types_match_finder(context);

        auto galena_type_matcher =
            cxxRecordDecl(eachOf(
                cxxRecordDecl(hasName("::galena::float4")).bind("galena_float4"),
                cxxRecordDecl(hasName("::galena::pixel_shader_position")).bind("galena_pixel_shader_position")
            ));
        galena_types_match_finder.add_multiple_matchers(galena_type_matcher,
                                                        std::make_tuple(
                                                            match_finder_wrapper::match_id<clang::CXXRecordDecl>("galena_float4"),
                                                            match_finder_wrapper::match_id<clang::CXXRecordDecl>("galena_pixel_shader_position")
                                                        ),
                                                        [this, &context](const clang::RecordDecl* float4_decl,
                                                                         const clang::RecordDecl* pixel_shader_position_decl) {
            if(float4_decl) {
                m_galena_float4_type = context.getRecordType(float4_decl)
                                            .getTypePtr()->getUnqualifiedDesugaredType();
            }

            if(pixel_shader_position_decl) {
                m_galena_pixel_shader_position_type = context.getRecordType(pixel_shader_position_decl)
                                                        .getTypePtr()->getUnqualifiedDesugaredType();
            }
        });

        galena_types_match_finder.match();


        auto function_name = std::string(m_function_signature.begin(),
                                         std::find(m_function_signature.begin(), m_function_signature.end(), '('));
        auto function_matcher = functionDecl(hasName(function_name), isDefinition()).bind("shader_function");

        auto function_parameters_str = std::string(std::find(m_function_signature.begin(), m_function_signature.end(), '(') + 1,
                                                   (std::find(m_function_signature.rbegin(), m_function_signature.rend(), ')') + 1).base());
        std::vector<std::string> function_parameter_str_set;
        boost::algorithm::split(function_parameter_str_set, function_parameters_str, [](char c) { return c == ','; });

        std::vector<clang::QualType> function_parameters;

        std::map<std::string, const clang::Type*> types = {
            { "galena::float4", m_galena_float4_type },
            { "galena::pixel_shader_position", m_galena_pixel_shader_position_type }
        };

        for(const auto& temp_func_param_str : function_parameter_str_set) {
            auto iter = types.find(boost::algorithm::trim_copy(temp_func_param_str));
            if(iter != types.end() && iter->second) {
                clang::Qualifiers qualifiers;
                function_parameters.emplace_back(clang::QualType(iter->second, qualifiers.getAsOpaqueValue()).getCanonicalType());
            }
            else {
                throw std::runtime_error("Unknown type.");
            }
        }

        const clang::FunctionDecl* ast_function = nullptr;

        match_finder_wrapper function_match_finder(context);
        function_match_finder.add_matcher<clang::FunctionDecl>(function_matcher, "shader_function",
                                                               [&ast_function, &function_parameters](const clang::FunctionDecl* func) {
            auto num_params = func->getNumParams();
            if(num_params != function_parameters.size()) return;

            for(unsigned int i = 0; i < num_params; ++i) {
                if(func->getParamDecl(i)->getType().getCanonicalType() != function_parameters[i]) return;
            };

            ast_function = func;
        });

        function_match_finder.match();

        if(!ast_function) throw std::runtime_error("Unable to locate function.");
        process_function(ast_function, m_function);
    }

    void process_function(const clang::FunctionDecl* ast_func, shader_model::function& function) {
        function.set_return_type(get_type_index_for_clang_type(ast_func->getReturnType()));
        function.set_name(ast_func->getName());

        for(unsigned int i = 0, num_params = ast_func->getNumParams(); i < num_params; ++i) {
            auto param = ast_func->getParamDecl(i);
            auto clang_param_name = param->getName();
            std::string param_name;
            if(clang_param_name == "") {
                param_name = function.get_name() + "_parameter_" + std::to_string(i);
            }
            else {
                param_name = clang_param_name;
            }

            function.add_parameter({
                get_type_index_for_clang_type(param->getType()),
                std::move(param_name)
            });
        }

        if(auto compound_stmt = llvm::dyn_cast<clang::CompoundStmt>(ast_func->getBody())) {
            process_function_body(compound_stmt, function);
        }
        else {
            throw new std::runtime_error("Unsupported function body type.");
        }
    }

    void process_function_body(const clang::CompoundStmt* ast_body, shader_model::function& function) {
        for(const auto stmt : ast_body->body()) {
            if(auto ast_return_stmt = llvm::dyn_cast<clang::ReturnStmt>(stmt)) {
                auto ast_return_stmt_value = ast_return_stmt->getRetValue();
                if(auto ast_construct_expr = llvm::dyn_cast<clang::CXXConstructExpr>(ast_return_stmt_value)) {
                    auto ast_constructor_arg_0 = ast_construct_expr->getArg(0);
                    if(get_type_index_for_clang_type(ast_construct_expr->getType()) != function.get_return_type()
                        || ast_construct_expr->getNumArgs() != 1) {
                        throw std::runtime_error("Unsupported return value conversion.");
                    }

                    const clang::Expr* ast_return_expr = nullptr;

                    if(auto ast_implicit_cast_expr = llvm::dyn_cast<clang::ImplicitCastExpr>(ast_constructor_arg_0)) {
                        if(get_type_index_for_clang_type(ast_implicit_cast_expr->getSubExpr()->getType()) == function.get_return_type()) {
                            ast_return_expr = ast_implicit_cast_expr->getSubExpr();
                        }
                    }
                    else if(clang::MaterializeTemporaryExpr::classof(ast_constructor_arg_0)) {
                        auto ast_temporary_expr = static_cast<const clang::MaterializeTemporaryExpr*>(ast_constructor_arg_0)->GetTemporaryExpr();
                        if(clang::ImplicitCastExpr::classof(ast_temporary_expr)) {
                            auto ast_sub_expr = static_cast<const clang::ImplicitCastExpr*>(ast_temporary_expr)->getSubExpr();
                            if(auto ast_temp_constructor_expr = llvm::dyn_cast<clang::CXXConstructExpr>(ast_sub_expr)) {
                                if(get_type_index_for_clang_type(ast_temp_constructor_expr->getType()) == function.get_return_type()) {
                                    if(ast_temp_constructor_expr->getNumArgs() == 1) {
                                        if(auto ast_temp_implicit_cast = llvm::dyn_cast<clang::ImplicitCastExpr>(ast_temp_constructor_expr->getArg(0))) {
                                            ast_return_expr = ast_temp_implicit_cast->getSubExpr();
                                        }
                                    }
                                }
                            }
                        }
                    }

                    if(!ast_return_expr) {
                        throw std::runtime_error("Unsupported return value conversion.");
                    }

                    function.add_operation(
                        shader_model::return_operation(
                            process_expression(ast_return_expr, function)));
                }
                else if(auto ast_init_list = llvm::dyn_cast<clang::InitListExpr>(ast_return_stmt_value)) {
                    shader_model::variable_construct_expression construct_expr(function.get_return_type());
                    for(auto ast_argument : *ast_init_list) {
                        construct_expr.add_argument(process_expression(static_cast<const clang::Expr*>(ast_argument), function));
                    }

                    function.add_operation(shader_model::return_operation(construct_expr));
                }
                else {
                    throw std::runtime_error("Unsupported return expression.");
                }
            }
            else {
                throw std::runtime_error("Unsupported statement.");
            }
        }
    }

    shader_model::expression process_expression(const clang::Expr* ast_expr, const shader_model::function& function) {
        if(auto ast_decl_ref_expr = llvm::dyn_cast<clang::DeclRefExpr>(ast_expr)) {
            if(auto ast_param_var_decl = llvm::dyn_cast<clang::ParmVarDecl>(ast_decl_ref_expr->getDecl())) {
                return shader_model::variable_reference_expression(
                            function.get_parameters()[ast_param_var_decl->getFunctionScopeIndex()]);
            }
        }
        else if(auto ast_floating_literal = llvm::dyn_cast<clang::FloatingLiteral>(ast_expr)) {
            if(&ast_floating_literal->getSemantics() == &llvm::APFloat::IEEEsingle) {
                return shader_model::literal_value_expression(ast_floating_literal->getValue().convertToFloat());
            }
        }

        throw std::runtime_error("Unsupported expression.");
    }

private:
    boost::typeindex::type_index get_type_index_for_clang_type(const clang::Type* clang_type) {
        clang_type = clang_type->getUnqualifiedDesugaredType();
        if(clang_type == m_galena_float4_type) {
            return boost::typeindex::type_id<galena::float4>();
        }
        else if(clang_type == m_galena_pixel_shader_position_type) {
            return boost::typeindex::type_id<galena::pixel_shader_position>();
        }

        throw std::runtime_error("Unrecognized type.");
    }

    boost::typeindex::type_index get_type_index_for_clang_type(clang::QualType clang_qual_type) {
        return get_type_index_for_clang_type(clang_qual_type.getTypePtr());
    }


    std::string m_function_signature;
    shader_model::function& m_function;

    const clang::Type* m_galena_float4_type = nullptr;
    const clang::Type* m_galena_pixel_shader_position_type = nullptr;
};


class clang_diagnostic_consumer : public clang::DiagnosticConsumer {
public:
    void HandleDiagnostic(clang::DiagnosticsEngine::Level diag_level,
                          const clang::Diagnostic& diag_info) override {
        if(diag_level == clang::DiagnosticsEngine::Level::Warning
            || diag_level == clang::DiagnosticsEngine::Level::Error
            || diag_level == clang::DiagnosticsEngine::Level::Fatal) {
            clang::SmallString<256> message;
            diag_info.FormatDiagnostic(message);
            std::cout << message.c_str() << std::endl;
        }
    }

    void finish() override {
        if(NumErrors != 0) {
            throw std::runtime_error("Shader code has error(s).");
        }
    }
};


class clang_frontend_action : public clang::ASTFrontendAction {
public:
    clang_frontend_action(std::string function_signature, shader_model::function& function)
        : m_function_signature(std::move(function_signature)), m_function(function) {}

    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance& compiler,
                                                          clang::StringRef file) override {
        return std::make_unique<clang_ast_consumer>(std::move(m_function_signature), m_function);
    }

private:
    std::string m_function_signature;
    shader_model::function& m_function;
};


class clang_frontend_action_factory : public clang::tooling::FrontendActionFactory {
public:
    clang_frontend_action_factory(std::string function_signature, shader_model::function& function)
        : m_function_signature(std::move(function_signature)), m_function(function) {}

    clang::FrontendAction *create() override {
        return new clang_frontend_action(m_function_signature, m_function);
    }

private:
    std::string m_function_signature;
    shader_model::function& m_function;
};


shader_compiler::shader_compiler() = default;
shader_compiler::~shader_compiler() = default;


shader_model::function shader_compiler::compile(std::string function_signature,
                                                const boost::filesystem::path& source_file,
                                                const boost::filesystem::path& galena_include_dir) {
    std::string galena_include_dir_str = "-I" + galena_include_dir.string();
    std::array<const char*, 3> args = {
        "--",
        "-std=c++14",
        galena_include_dir_str.c_str()
    };

    int argc = args.size();
    std::unique_ptr<clang::tooling::FixedCompilationDatabase> compilations(
        clang::tooling::FixedCompilationDatabase::loadFromCommandLine(argc, args.data()));

    clang::tooling::ClangTool tool(*compilations, { source_file.string() });
    auto diagnostic_consumer = std::make_unique<clang_diagnostic_consumer>();
    tool.setDiagnosticConsumer(diagnostic_consumer.get());

    shader_model::function function;
    auto frontend_action_factory = std::make_unique<clang_frontend_action_factory>(std::move(function_signature), function);
    tool.run(frontend_action_factory.get());

    return function;
}


}
