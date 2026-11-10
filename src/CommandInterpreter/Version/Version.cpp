#include "Version.hpp"
#include "../../Data/Config.hpp"
#include <fmt/color.h>
#include <fmt/base.h>

void showVersion(){
    const char logo[] = {
        #embed "Logo.txt"
        ,0
    };
    fmt::println(fmt::emphasis::bold, "{}", logo);
    fmt::println(
        fmt::fg(fmt::color::burly_wood) | fmt::emphasis::bold,
        "Xylose version: {}.{}.{}", cfg::VERSION_MAJOR, cfg::VERSION_MINOR, cfg::VERSION_PATCH
    );
}
