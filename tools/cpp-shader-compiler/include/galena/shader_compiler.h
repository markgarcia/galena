#pragma once

#include <galena/shader/shader_model.h>

#include <boost/filesystem.hpp>


namespace galena {


class shader_compiler {
public:
    galena::shader_model::function compile(const std::string& function_name,
                                           const boost::filesystem::path& source_file,
                                           const boost::filesystem::path& galena_include_dir);

private:
    friend class clang_cursor_visitor;
};


}
