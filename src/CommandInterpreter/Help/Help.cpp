#include "Help.hpp"
#include <fmt/base.h>

void showHelp() {
    const char msg[] = {
        #embed "./Help.txt"
        ,0
    };
    fmt::println("{}", msg);
}
