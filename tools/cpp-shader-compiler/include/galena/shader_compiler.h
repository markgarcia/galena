#pragma once

#include <galena/shader/shader_model.h>

#include <boost/filesystem.hpp>


namespace galena {


class shader_compiler {
public:
    shader_compiler(const std::string& function_name,
                    const boost::filesystem::path& source_file,
                    const boost::filesystem::path& galena_include_dir);

    galena::shader_model get_shader_model();
};


}
