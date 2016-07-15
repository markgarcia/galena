#pragma once

#include <galena/shader/shader_model.h>

#include <boost/filesystem.hpp>
#include <memory>


namespace galena {


class shader_compiler {
public:
    shader_compiler();
    ~shader_compiler();

    galena::shader_model::function compile(const std::string& function_name,
                                           const boost::filesystem::path& source_file,
                                           const boost::filesystem::path& galena_include_dir);

private:
    friend class clang_cursor_visitor;

    struct galena_shader_types;
    std::unique_ptr<galena_shader_types> m_shader_types;
};


}
