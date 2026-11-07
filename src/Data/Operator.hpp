#include <cstdint>
#include <string_view>

struct Operator{
    std::string_view name;
    enum InputType : std::uint8_t{
        LEFT = 1,
        RIGHT = 2,
        BOTH = 3
    }input_type = BOTH;
};
