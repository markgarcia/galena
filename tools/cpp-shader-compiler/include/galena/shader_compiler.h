#pragma once

#include "galena/shader_model.h"

#include <boost/filesystem.hpp>


namespace galena {


class BOOST_SYMBOL_EXPORT shader_compiler {
public:
    shader_compiler();
    ~shader_compiler();

    galena::shader_model::function compile(std::string function_signature,
                                           const boost::filesystem::path& source_file,
                                           const boost::filesystem::path& galena_include_dir);
};


}
