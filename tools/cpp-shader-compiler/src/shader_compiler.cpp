#include "galena/shader_compiler.h"

#include "galena/shader/shader.h"

#include <llvm/DebugInfo/Symbolize/Symbolize.h>
#include <clang-c/Index.h>

#include <boost/optional.hpp>
#include <boost/operators.hpp>
#include <iostream>
#include <algorithm>


namespace galena {


class clang_string
    : boost::equality_comparable<clang_string, std::string>,
      boost::equality_comparable<clang_string, const char*>{
    CXString m_string;
    bool m_has_string = true;

public:
    clang_string(CXString string) : m_string(string) {}
    ~clang_string() { if(m_has_string) clang_disposeString(m_string); }

    clang_string(const clang_string&) = delete;
    clang_string& operator=(const clang_string&) = delete;

    clang_string(clang_string&& rhs) {
        m_string = rhs.m_string;
        rhs.m_has_string = false;
    }

    clang_string& operator=(clang_string&& rhs) {
        m_string = rhs.m_string;
        m_has_string = true;
        rhs.m_has_string = false;

        return *this;
    }

    const char* c_str() const { return clang_getCString(m_string); }
};


bool operator==(const clang_string& lhs, const std::string& rhs) { return lhs.c_str() == rhs; }
bool operator==(const clang_string& lhs, const char* rhs) { return std::strcmp(lhs.c_str(), rhs) == 0; }


template<typename clang_type, void (*deleter)(clang_type)>
class clang_resource {
    clang_type m_resource;
    bool m_has_resource = true;

public:
    clang_resource(clang_type resource) : m_resource(resource) {}
    ~clang_resource() { if(m_has_resource) deleter(m_resource); }

    clang_resource(const clang_resource&) = delete;
    clang_resource& operator=(const clang_resource&) = delete;

    clang_resource(clang_resource&& rhs) {
        m_resource = rhs.m_resource;
        rhs.m_has_resource = false;
    }

    clang_resource& operator=(clang_resource&& rhs) {
        m_resource = rhs.m_resource;
        m_has_resource = true;
        rhs.m_has_resource = false;

        return *this;
    }

    clang_type& get() { return m_resource; }
    operator clang_type&() { return m_resource; }
};


struct shader_compiler::galena_shader_types {
    CXType float4_type;
};


class clang_cursor_visitor {
public:
    struct function_visitor_data {
        shader_compiler& compiler;
        const std::string& required_function_name;
        boost::optional<shader_model::function> function;
    };

    static CXChildVisitResult function_visitor(CXCursor cursor, CXCursor parent, CXClientData client_data);

    static CXChildVisitResult galena_shader_types_visitor(CXCursor cursor, CXCursor parent, CXClientData client_data);
};


shader_compiler::shader_compiler() = default;
shader_compiler::~shader_compiler() = default;


shader_model::function shader_compiler::compile(const std::string& function_name,
                                                const boost::filesystem::path& source_file,
                                                const boost::filesystem::path& galena_include_dir) {
    clang_resource<CXIndex, clang_disposeIndex> index = clang_createIndex(0, 0);

    std::string galena_include_dir_str = "-I" + galena_include_dir.string();
    std::array<const char*, 2> args = {
        "-std=c++14",
        galena_include_dir_str.c_str()
    };

    clang_resource<CXTranslationUnit, clang_disposeTranslationUnit> translation_unit(
        clang_parseTranslationUnit(
            index, source_file.generic_string().c_str(),
            args.data(), args.size(), NULL, 0, CXTranslationUnit_None));

    if(!translation_unit) {
        throw std::invalid_argument("Invalid source file.");
    }

    bool has_error = false;
    for(unsigned i = 0; i < clang_getNumDiagnostics(translation_unit); i++) {
        clang_resource<CXDiagnostic, clang_disposeDiagnostic> diagnostic = clang_getDiagnostic(translation_unit, i);
        auto severity = clang_getDiagnosticSeverity(diagnostic);

        if(severity == CXDiagnostic_Error || severity == CXDiagnostic_Fatal) {
            has_error = true;
        }

        clang_string diagnosticMessage = clang_formatDiagnostic(diagnostic, CXDiagnostic_DisplaySourceLocation);
        std::cout << diagnosticMessage.c_str() << std::endl;
    }

    if(has_error) {
        throw std::runtime_error("Source file has error(s).");
    }

    auto cursor = clang_getTranslationUnitCursor(translation_unit);

    m_shader_types = std::make_unique<galena_shader_types>();

    // Initialize with invalid type
    auto invalid_type = clang_getCursorType(cursor);
    m_shader_types->float4_type = invalid_type;

    clang_visitChildren(cursor, clang_cursor_visitor::galena_shader_types_visitor, m_shader_types.get());

    clang_cursor_visitor::function_visitor_data visitor_data { *this, function_name, boost::none };
    clang_visitChildren(cursor, clang_cursor_visitor::function_visitor, &visitor_data);
    if(!visitor_data.function) throw std::runtime_error("Function not found.");

    return std::move(visitor_data.function.get());
}


CXChildVisitResult clang_cursor_visitor::galena_shader_types_visitor(CXCursor cursor, CXCursor parent, CXClientData client_data) {
    if(clang_getCursorKind(cursor) == CXCursor_Namespace) {
        clang_string name = clang_getCursorSpelling(cursor);
        if(name != "galena") return CXChildVisit_Continue;

        clang_visitChildren(cursor, [](CXCursor cursor, CXCursor parent, CXClientData client_data) {
            if(clang_getCursorKind(cursor) == CXCursor_StructDecl) {
                auto& shader_types = *static_cast<shader_compiler::galena_shader_types*>(client_data);

                clang_string name = clang_getCursorSpelling(cursor);
                if(name == "float4") {
                    shader_types.float4_type = clang_getCursorType(cursor);
                }
            }

            return CXChildVisit_Continue;
        }, client_data);

        return CXChildVisit_Break;
    }

    return CXChildVisit_Continue;
}


CXChildVisitResult clang_cursor_visitor::function_visitor(CXCursor cursor, CXCursor parent, CXClientData client_data) {
    auto& data = *static_cast<clang_cursor_visitor::function_visitor_data*>(client_data);

    // Find desired function
    if(clang_getCursorKind(cursor) == CXCursor_FunctionDecl) {
        clang_string func_mangled_name = clang_Cursor_getMangling(cursor);
        auto func_name = llvm::symbolize::LLVMSymbolizer::DemangleName(func_mangled_name.c_str(), nullptr);
        if(data.required_function_name != func_name) return CXChildVisit_Continue;

        auto shader_func_name = func_name.substr(0, func_name.find_first_of('('));
        auto iter = shader_func_name.begin();

        while(iter != shader_func_name.end()) {
            auto colon = std::adjacent_find(iter, shader_func_name.end(),
                                            [](auto first, auto second) { return first == second && first == ':'; });
            if(colon == shader_func_name.end()) break;
            *colon = '_';
            iter = std::copy(colon + 2, std::find(colon + 2, shader_func_name.end(), ':'), colon + 1);
            shader_func_name.erase(shader_func_name.end() - 1);
        }

        data.function = shader_model::function();
        data.function->set_name(std::move(shader_func_name));

        auto definition = clang_getCursorDefinition(cursor);

        // Get parameters
        auto num_params = clang_Cursor_getNumArguments(definition);
        for(decltype(num_params) i = 0; i < num_params; ++i) {
            auto param = clang_Cursor_getArgument(definition, static_cast<unsigned int>(i));
            auto param_clang_type = clang_getCursorType(param);

            boost::typeindex::type_index param_type;
            if(clang_equalTypes(param_clang_type, data.compiler.m_shader_types->float4_type)) {
                param_type = boost::typeindex::type_id<galena::float4>();
            }

            clang_string param_name = clang_getCursorSpelling(param);
            if(param_name == "") {
                data.function->add_parameter({
                    param_type,
                    data.function->get_name() + "_parameter_" + std::to_string(data.function->get_parameters().size())
                });
            }
            else {
                data.function->add_parameter({ param_type, param_name.c_str() });
            }
        }


        return CXChildVisit_Break;
    }

    return CXChildVisit_Recurse;
}


}
