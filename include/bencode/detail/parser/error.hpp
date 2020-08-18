#pragma once

#include <exception>
#include <system_error>
#include <string>

#include <gsl/gsl>
#include <fmt/format.h>

#include "bencode/detail/symbol.hpp"
#include "bencode/detail/bencode_type.hpp"

namespace bencode {

/// Error code enumeration for parsing errors.
enum class parser_errc {
//    no_error = 0,
    expected_colon = 1,
    expected_digit,
    expected_dict_key_or_end,
    expected_end,
    expected_value,
    expected_dict_value,
    expected_integer_start_token,
    negative_string_length,
    unexpected_end,
    unexpected_eof,
    integer_overflow,
    leading_zero,
    negative_zero,
    recursion_depth_exceeded,
    value_limit_exceeded,
    internal_error,
};

/// Converts ec to a string.
/// @param ec an error code
/// @returns String representation of an error code.
constexpr std::string_view to_string(parser_errc ec) {
    switch (ec) {
    case parser_errc::expected_colon:
        return "expected string length separator ':'";
    case parser_errc::expected_digit:
        return "expected digit";
    case parser_errc::expected_dict_key_or_end:
        return "expected a dict key or end token";
    case parser_errc::expected_end:
        return "expected end token";
    case parser_errc::expected_value:
        return "expected begin of a bvalue ('i', 'l', 'd', or a digit)";
    case parser_errc::expected_dict_value:
        return "missing bvalue for key in dict";
    case parser_errc::unexpected_end:
        return "mismatched end token";
    case parser_errc::unexpected_eof:
        return "unexpected end of input";
    case parser_errc::integer_overflow:
        return "integer overflow";
    case parser_errc::leading_zero:
        return "leading zero(s) is forbidden";
    case parser_errc::negative_zero:
        return "negative zero is forbidden";
    case parser_errc::recursion_depth_exceeded:
        return "nested list/dict depth exceeded";
    case parser_errc::value_limit_exceeded:
        return "bvalue limit exceeded";
    case parser_errc::negative_string_length:
        return "invalid string length";
    case parser_errc::internal_error:
        return "internal error";
    default:
        return "(unrecognised error)";
    };
}

struct parser_category : std::error_category
{
    const char* name() const noexcept override {
        return "parser error";
    }

    std::string message(int ev) const override
    {
        return std::string(to_string(static_cast<bencode::parser_errc>(ev)));
    }
};


inline std::error_code make_error_code(parser_errc e)
{
    return {static_cast<int>(e), bencode::parser_category()};
}

} // namespace bencode

namespace std {
template <> struct is_error_code_enum<bencode::parser_errc> : std::true_type{};
}

namespace bencode {


/// Exception class for parser errors.
class parse_error : public std::runtime_error {
public:
     parse_error(parser_errc ec, std::size_t pos, std::optional<bencode_type> context = std::nullopt)
            : std::runtime_error(make_what_msg(ec, pos, context))
            , position_(pos)
            , context_(context)
    {}

    parse_error(const parse_error&) noexcept = default;
    parse_error& operator=(const parse_error&) noexcept = default;

    /// The byte index of the last valid character in the input file.
    ///
    /// @note For an input with n bytes, 1 is the index of the first character and
    ///       n+1 is the index of the terminating null byte or the end of file.
    ///
    std::size_t position() const noexcept { return position_; }

    /// Retu
    std::optional<bencode_type> context() const { return context_; }

private:
    parse_error(const char* what, std::size_t position, std::optional<bencode_type> context)
            : std::runtime_error(what)
            , position_(position)
            , context_(context)
    { };

    static std::string make_what_msg(parser_errc ec, std::size_t pos, std::optional<bencode_type> context)
    {
        using namespace std::string_literals;
        std::string what = fmt::format("parse error: {}", position_string(pos));
        auto what_inserter = std::back_inserter(what);
        if (context) {
            fmt::format_to(what_inserter, ", while parsing: \"{}\"", to_string(*context));
        }
        fmt::format_to(what_inserter, ": {}", to_string(ec));
        return what;
    }

    static std::string position_string(std::size_t pos)
    {
        return fmt::format("invalid character at position {}", pos);
    }

private:
    std::size_t position_;
    std::optional<bencode_type> context_;
};


} // namespace bencode
