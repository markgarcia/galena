#include "galena/source_location.h"

#include <llvm/DebugInfo/Symbolize/Symbolize.h>
#include <boost/dll.hpp>


namespace galena {


function_location locate_function(uint64_t func_address) {
    llvm::symbolize::LLVMSymbolizer symbolizer;
    auto line_info = symbolizer.symbolizeCode(boost::dll::program_location().generic_string(), func_address);
    if(!line_info) {
        throw std::runtime_error("Unable to retrieve source code of function. "
                                 "Error: " + line_info.getError().message());
    }

    return {
        line_info.get().FileName,
        line_info.get().FunctionName,
        line_info.get().Line,
        line_info.get().Column
    };
}


}
