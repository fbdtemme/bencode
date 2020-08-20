#pragma once

#include <concepts>
#include <gsl/gsl>
#include <immintrin.h>
#include <nonstd/expected.hpp>
#include <ranges>

#include <bencode/detail/concepts.hpp>
#include <bencode/detail/symbol.hpp>
#include <bencode/detail/utils.hpp>
#include "parsing_error.hpp"

/// @file Shared code between parsers.

namespace bencode {

/// Pass safety options to a parser.
struct parser_options
{
    /// Maximum number of nested bencode objects.
    std::uint32_t recursion_limit = 1024;
    /// Maximum number of values to parse.
    std::uint32_t value_limit = 8192;
};
}

namespace bencode::detail {

namespace rng = std::ranges;

/// Enum type of the current parse context in nested objects for list and dicts.
enum class parser_state : char {
    expect_value,
    expect_list_value,
    expect_dict_key,
    expect_dict_value
};

/// Parse an integer from a string.
///
/// Try to decode a single integeral bvalue from a character sequence.
/// Set begin to the first character after the decoded integer.
/// Return the bvalue or an error code as a std::expected object.
///
/// @tparam IntT the type to decode to
/// @param  first start of the character sequence
/// @param  last end of the character sequence
/// @returns `expected` bvalue with the decoded integer or an error code
template <typename IntT, typename It>
    requires std::integral<IntT> && std::input_iterator<It>
constexpr nonstd::expected<IntT, parsing_errc>
parse_integer(It& begin, It end) noexcept
{
    constexpr auto max_digits = std::numeric_limits<IntT>::digits10;
    bool negative = false;

    auto it = begin;
    IntT value {};

    if constexpr (std::signed_integral<IntT>) {
        if (*it == '-') {
            negative = true;
            ++it;
        }
    }
    // "" or "-" or "-{non digit}"
    if (it == end || !std::isdigit(*it)) [[unlikely]] {
        return nonstd::make_unexpected(parsing_errc::expected_digit);
    }

    // do not advance here since we must read the zero again
    const bool leading_zero = (*it == symbol::zero);
    int digits = 0;

    for (; digits < max_digits && (it != end) && std::isdigit(*it); ++digits, ++it)
    {
        value = 10 * value + static_cast<unsigned>(*it)-'0';
    }

    // use integer safe math to check for overflow if we have only one digit precision left
    if (digits == max_digits && it != end && std::isdigit(*it)) [[unlikely]]
    {
        bool overflow =
                __builtin_mul_overflow(value, 10, &value) |
                __builtin_add_overflow(value, *it++-'0', &value);

        if (overflow) [[unlikely]] {
            return nonstd::make_unexpected(parsing_errc::integer_overflow);
        }
    }

    if (leading_zero && value != 0) [[unlikely]] {
        return nonstd::make_unexpected(parsing_errc::leading_zero);
    }

    if constexpr (std::is_signed_v<IntT>) {
        if (negative && value == 0) [[unlikely]] {
            return nonstd::make_unexpected(parsing_errc::negative_zero);
        }
        if (negative) { value = -value; }
    }
    begin = it;
    return value;
}

/// @copydoc bencode::detail::parse_integer
template <typename IntT, typename It>
    requires std::integral<IntT> && std::input_iterator<It>
constexpr nonstd::expected<IntT, parsing_errc>
parse_integer(It&& begin, It end) noexcept
{
    auto b = begin;
    return parse_integer<IntT>(b, end);
}

/// Decode a bencoded integer.
///
/// Try to decode one integer token (eg. i666e) from a char sequence
/// given by a pair of iterators.
///
/// @param first Start of the char sequence.
/// @param last End of the char sequence.
/// @param ec Error code in case of parsing errors.
/// @return Returns an optional pair of the bvalue and an iterator pointing to the
///         first non decoded character in the input sequence.
///         When the optional is empty a parsing error occurred.
template<std::integral IntT = std::int64_t, std::input_iterator It>
constexpr nonstd::expected<IntT, parsing_errc>
bdecode_integer(It& begin, It end) noexcept
{
    auto it = begin;

    if (it == end) [[unlikely]] {
        return nonstd::make_unexpected(parsing_errc::unexpected_eof);
    }
    if (*it != 'i') [[unlikely]] {
        return nonstd::make_unexpected(parsing_errc::expected_integer_start_token);
    }
    // skip 'i'
    ++it;
    auto result = parse_integer<std::int64_t>(it, end);

    // pass possible errors from parse_integer
    if (!result) [[unlikely]] {
        return nonstd::make_unexpected(result.error());
    }
    const auto value = *result;

    // verify the integer is correctly terminated with the "e"
    if (it == end || *it != symbol::end) [[unlikely]] {
        return nonstd::make_unexpected(parsing_errc::expected_end);
    }

    ++it;
    begin = it;
    return value;
}

// Decode a bencoded integer.
template <typename IntT = std::int64_t, typename It>
    requires std::integral<IntT> && std::input_iterator<It> &&
            (std::is_rvalue_reference_v<It> || std::is_pointer_v<It>)
constexpr nonstd::expected<IntT, parsing_errc>
bdecode_integer(It&& begin, It end) noexcept
{
    auto b = begin;
    return bdecode_integer<IntT>(b, end);
}


template<typename StringT = std::string, typename It>
    requires std::input_iterator<It>
inline nonstd::expected<StringT, parsing_errc>
bdecode_string(It& begin, It end)
{
    auto it = begin;

    if (it == end) [[unlikely]] {
        return nonstd::make_unexpected(parsing_errc::unexpected_eof);
    }
    if (*it == '-') [[unlikely]] {
        return nonstd::make_unexpected(parsing_errc::negative_string_length);
    }

    const auto size_or_err = parse_integer<std::size_t>(it, end);
    if (!size_or_err) [[unlikely]] {
        return nonstd::make_unexpected(size_or_err.error());
    }
    const auto size = size_or_err.value();

    if (it == end || *it != symbol::colon) [[unlikely]] {
        return nonstd::make_unexpected(parsing_errc::expected_colon);
    }
    ++it;

    StringT value{};

    // reserve space if the output supports it
    if constexpr (detail::has_reserve_member<StringT>) {
        value.reserve(size);
    }
    if constexpr (std::contiguous_iterator<It>) {
        if (std::next(it, size) > end) [[unlikely]] {
            return nonstd::make_unexpected(parsing_errc::unexpected_eof);
        }
        value = StringT(it, std::next(it, size));
        std::advance(it, size);
    }
    else {
        for (size_t idx = 0; idx < size; ++it, ++idx) {
            if (it == end) [[unlikely]] {
                return nonstd::make_unexpected(parsing_errc::unexpected_eof);
            }
            value.push_back(*it);
        }
    }
    begin = it;
    return value;
}


// Decode a bencoded integer.
template<typename StringT = std::string, typename It>
    requires std::input_iterator<It> &&
            (std::is_rvalue_reference_v<It> || std::is_pointer_v<It>)
constexpr nonstd::expected<StringT, parsing_errc>
bdecode_string(It&& begin, It end) noexcept
{
    auto b = begin;
    return bdecode_string<StringT>(b, end);
}

struct bdecode_string_token_result
{
    std::uint32_t offset;
    std::uint32_t size;
};


template <typename It>
    requires std::contiguous_iterator<It>
constexpr nonstd::expected<bdecode_string_token_result, parsing_errc>
bdecode_string_token(It& begin , It end) noexcept
{
    auto it = begin;

    if (it == end) [[unlikely]] {
        return nonstd::make_unexpected(parsing_errc::unexpected_eof);
    }
    if (*it == '-') [[unlikely]] {
        return nonstd::make_unexpected(parsing_errc::negative_string_length);
    }

    const auto init_pos = static_cast<std::uint32_t>(std::distance(begin, it));
    auto result = parse_integer<std::size_t>(it, end);

    if (!result) [[unlikely]] {
        return nonstd::make_unexpected(result.error());
    }
    const auto size = *result;

    if (it == end || *it != symbol::colon) [[unlikely]] {
        return nonstd::make_unexpected(parsing_errc::expected_colon);
    }
    ++it;

    Ensures(size <= std::numeric_limits<std::uint32_t>::max());

    const auto prefix_length = static_cast<std::uint32_t>(std::distance(begin, it));

    if (std::next(it, size) > end) [[unlikely]] {
        return nonstd::make_unexpected(parsing_errc::unexpected_eof);
    }
    std::advance(it, size);
    begin = it;
    return bdecode_string_token_result(prefix_length, size);
}


template <typename StringViewT = std::string_view, typename It>
    requires std::contiguous_iterator<It> &&
            (std::is_rvalue_reference_v<It> || std::is_pointer_v<It>)
constexpr nonstd::expected<bdecode_string_token_result, parsing_errc>
bdecode_string_token(It&& begin, It end) noexcept
{
    auto b = begin;
    return bdecode_string_token(b, end);
}

}