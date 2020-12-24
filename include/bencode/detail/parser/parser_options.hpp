#pragma once
#include <cstdint>

namespace bencode {

/// Pass safety options to a parser.
struct parser_options
{
    /// Maximum number of nested bencode objects.
    std::size_t recursion_limit = 1024;
    /// Maximum number of values to parse.
    std::size_t value_limit = 1UL << 32;
};

}
