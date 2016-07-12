#pragma once

#include <galena/shader/shader_model.h>

#include <clang-c/Index.h>

#include <boost/filesystem.hpp>


namespace galena {


class shader_transpiler {
public:
    shader_transpiler(const std::string& function_name,
                      const boost::filesystem::path& source_file,
                      const boost::filesystem::path& galena_include_dir);

    galena::shader_model get_shader_model();

private:
    static CXChildVisitResult cursor_visitor(CXCursor cursor, CXCursor parent, CXClientData client_data);
};


}
