#pragma once

#include <cstdint>

namespace bencode {

/// Pass safety options to a parser.
struct parser_options
{
    /// Maximum number of nested bencode objects.
    std::uint32_t recursion_limit = 1024;
    /// Maximum number of values to parse.
    std::uint32_t value_limit = 8192;
};

namespace detail
{
/// Enum type of the current parse context in nested objects for list and dicts.
enum class parser_state : char {
    expect_value,
    expect_list_value,
    expect_dict_key,
    expect_dict_value
};
}

}