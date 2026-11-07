#include "Lexer/Lexer.hpp"
#include "Parser/Parser.hpp"
#include "Data/Diagnostics.hpp"
#include <exception>
#include <fmt/printf.h>
#include <fmt/color.h>

int main(){
    try{
        std::string source =
            "type  A<T: fn(..) -> any> = T #a\n"
            "fn b -> || A<fn(i32, u32, i32) -> u64>\n";
        Lexer lexer(source);
        auto tokens = lexer.splitIntoTokens();
        Parser parser{tokens};

        parser.splitIntoGlobalTokenRanges();
        parser.parseGlobals();
    }
    catch(const std::exception& e){
        fmt::print(diag::ERROR, "Error: ");
        fmt::println("{}", e.what());
    }
}
