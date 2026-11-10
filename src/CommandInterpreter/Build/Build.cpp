#include "Build.hpp"
#include "../../Lexer/Lexer.hpp"
#include "../../Parser/Parser.hpp"
#include "../../Data/Diagnostics.hpp"
#include "fmt/color.h"
#include <cstdlib>
#include <fmt/base.h>
#include <exception>
#include <fstream>

void build(int argc, char **argv) {
    try{
        std::string path = "main.xylo";
        if(argc > 2) {
            path = argv[2];
        }
        std::ifstream file;
        file.open(path, std::ios::ate);
        if(!file.is_open()){
            fmt::println(diag::ERROR, "Could not find/open file: {}.", path);
            std::exit(-1);
        }
        std::size_t size = file.tellg();
        std::string src;
        src.resize(size);
        file.seekg(0, std::ios::beg);
        file.read(src.data(), size);
        DIAG_DBG(
            fmt::println(diag::DIAGNOSTIC_HEADER, "--{}--", path);
            fmt::println("{}", src);
        )
        Lexer lex(src);
        auto tokens = lex.splitIntoTokens();
        Parser par(tokens);
        par.groupIntoGlobalTokenRanges();
        par.parseGlobals();
    } catch(const std::exception& e) {
        fmt::print(diag::ERROR, "Error: ");
        fmt::println("{}", e.what());
    }
}
