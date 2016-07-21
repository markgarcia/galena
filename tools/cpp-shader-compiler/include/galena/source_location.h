#pragma once

#include <boost/filesystem.hpp>


namespace galena {


struct function_location {
    boost::filesystem::path source_file;
    std::string function_signature;
    unsigned int line;
    unsigned int column;
};


function_location BOOST_SYMBOL_EXPORT locate_function(uint64_t func_address);


}
