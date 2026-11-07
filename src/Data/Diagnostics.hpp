#pragma once
#include <fmt/color.h>
#include <string_view>
#ifdef NDEBUG
#define DIAG_DBG(x)
#else
#define DIAG_DBG(x) x
#endif

namespace diag {
    constexpr auto DIAGNOSTIC = fmt::emphasis::bold | fmt::fg(fmt::color::dark_cyan);
    constexpr auto DIAGNOSTIC_HEADER = fmt::emphasis::bold | fmt::fg(fmt::color::cyan);
    constexpr auto ERROR = fmt::emphasis::bold | fmt::emphasis::italic | fmt::fg(fmt::color::red);
    constexpr std::string_view UNKNOWN_TYPE = "Unknown";
}
