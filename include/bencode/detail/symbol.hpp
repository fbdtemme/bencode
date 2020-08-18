#pragma once

#include <cstdint>
#include <cctype>
#include <iosfwd>
#include <string_view>
#include <compare>

namespace bencode {
namespace symbol {

// use non-scoped enum to allow implicit conversion to char
enum symbol : char {
    begin_integer = 'i',
    begin_list    = 'l',
    begin_dict    = 'd',
    end           = 'e',
    colon         = ':',
    minus         = '-',
    zero          = '0'
};

} // namespace symbol

namespace detail {

/// Tag type for digits.
struct digit_symbol_type {
    explicit constexpr digit_symbol_type() = default;

    constexpr bool operator==(const char c) const noexcept
    { return static_cast<unsigned>(c) - '0' < 10; }
};

} // namespace detail

namespace symbol {
inline constexpr detail::digit_symbol_type digit {};
}

namespace detail {

struct value_symbol_type {
    explicit constexpr value_symbol_type() = default;

    constexpr bool operator==(char c) const noexcept
    {
        return (c == symbol::begin_integer
                || c == symbol::digit
                || c == symbol::begin_list
                || c == symbol::begin_dict);
    }
};

} // namespace detail

namespace symbol {
inline constexpr detail::value_symbol_type value{};
}

} // namespace bencode::symbol
