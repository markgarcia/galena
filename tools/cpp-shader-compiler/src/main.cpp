#include <clang-c/Index.h>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <string>
#include <array>

namespace po = boost::program_options;


int main(int argc, const char* argv[])
{
    po::options_description desc("Allowed options");
    desc.add_options()
        ("file", "File containing C++ shader code to be compiled.")
        ("galena-root", "Root directory of galena library.")
        ("help", "");

    po::positional_options_description options_desc;
    options_desc.add("file", -1);

    po::variables_map options;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(options_desc).run(), options);
    po::notify(options);

    if(options.count("help")) {
        std::cout << desc << std::endl;
        return 0;
    }

    if(!options.count("file")) {
        std::cerr << "File is required." << std::endl;
        return 1;
    }

    auto input_file = options["file"].as<std::string>();

    boost::filesystem::path galena_include_dir;
    if(options.count("galena-root")) {
        galena_include_dir = options["galena-root"].as<std::string>();
    }
    else {
        // Try using working directory as starting point
        auto current_path = boost::filesystem::current_path();

        if(*(current_path.rbegin()) == "cpp-shader-compiler") {
            galena_include_dir =
                current_path
                .parent_path()      // tools
                .parent_path();     // galena root
        }
        else {
            std::cerr << "Unable to find galena root directory." << std::endl;
            return 1;
        }
    }

    galena_include_dir /= "include";

    CXIndex index = clang_createIndex(0, 0);

    std::string galena_include_dir_str = "-I" + galena_include_dir.string();
    std::array<const char*, 2> args = {
        "-std=c++14",
        galena_include_dir_str.c_str()
    };
    auto tu = clang_parseTranslationUnit(index, input_file.c_str(), args.data(), args.size(), NULL, 0, CXTranslationUnit_None);

    if(!tu) {
        std::cerr << "Invalid source file." << std::endl;
        return 1;
    }

    auto diagnosticCount = clang_getNumDiagnostics(tu);
    for(unsigned i = 0; i < diagnosticCount; i++) {
        auto diagnostic = clang_getDiagnostic(tu, i);
        auto location = clang_getDiagnosticLocation(diagnostic);

        CXFile source_file;
        unsigned int line, column;
        clang_getSpellingLocation(location, &source_file, &line, &column, NULL);

        auto source_filename = clang_getFileName(source_file);
        auto diagnostic_message = clang_getDiagnosticSpelling(diagnostic);

        std::cout << clang_getCString(source_filename) << ":" << line << ":" << column << ": "
                  << "error: " << clang_getCString(diagnostic_message) << std::endl;
    }
}
