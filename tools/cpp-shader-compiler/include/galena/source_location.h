#pragma once

#include <boost/filesystem.hpp>


namespace galena {


struct function_location {
    boost::filesystem::path source_file;
    std::string function_name;
    unsigned int line;
    unsigned int column;
};


function_location locate_function(uint64_t func_address);


}
