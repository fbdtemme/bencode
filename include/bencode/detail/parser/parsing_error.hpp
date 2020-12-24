#pragma once

#include <exception>
#include <system_error>
#include <string>

#include <gsl-lite/gsl-lite.hpp>
#include <fmt/format.h>

#include "bencode/detail/symbol.hpp"
#include "bencode/detail/bencode_type.hpp"
#include "bencode/detail/exception.hpp"

namespace bencode {

/// Error code enumeration for parsing errors.
enum class parsing_errc {
//    no_error = 0,
    expected_colon = 1,
    expected_digit,
    expected_dict_key_or_end,
    expected_list_value_or_end,
    expected_end,
    expected_value,
    expected_dict_value,
    expected_integer_start_token,
    negative_string_length,
    unexpected_end,
    unexpected_eof,
    result_out_of_range,
    leading_zero,
    negative_zero,
    recursion_depth_exceeded,
    value_limit_exceeded,
    internal_error,
};

/// Converts ec to a string.
/// @param ec an error code
/// @returns String representation of an error code.
constexpr std::string_view to_string(parsing_errc ec) {
    switch (ec) {
    case parsing_errc::expected_colon:
        return "expected string length separator ':'";
    case parsing_errc::expected_digit:
        return "expected digit";
    case parsing_errc::expected_dict_key_or_end:
        return "expected a dict key or end token";
    case parsing_errc::expected_list_value_or_end:
        return "expected a list value or end token";
    case parsing_errc::expected_end:
        return "expected end token";
    case parsing_errc::expected_value:
        return "expected begin of a bvalue ('i', 'l', 'd', or a digit)";
    case parsing_errc::expected_dict_value:
        return "missing bvalue for key in dict";
    case parsing_errc::unexpected_end:
        return "mismatched end token";
    case parsing_errc::unexpected_eof:
        return "unexpected end of input";
    case parsing_errc::result_out_of_range:
        return "result out of range of destination type";
    case parsing_errc::leading_zero:
        return "leading zero(s) is forbidden";
    case parsing_errc::negative_zero:
        return "negative zero is forbidden";
    case parsing_errc::recursion_depth_exceeded:
        return "nested object depth exceeded";
    case parsing_errc::value_limit_exceeded:
        return "value limit exceeded";
    case parsing_errc::negative_string_length:
        return "invalid string length";
    case parsing_errc::internal_error:
        return "internal error";
    default:
        return "(unrecognised error)";
    };
}

struct parsing_category : std::error_category
{
    const char* name() const noexcept override
    {
        return "parsing error";
    }

    std::string message(int ev) const override
    {
        return std::string(to_string(static_cast<bencode::parsing_errc>(ev)));
    }
};


inline std::error_code make_error_code(parsing_errc e)
{
    return {static_cast<int>(e), bencode::parsing_category()};
}

} // namespace bencode

namespace std {
template <> struct is_error_code_enum<bencode::parsing_errc> : std::true_type{};
}

namespace bencode {


/// Exception class for parser errors.
class parsing_error : public exception {
public:
     parsing_error(parsing_errc ec, std::size_t pos, std::optional<bencode_type> context = std::nullopt)
            : exception(make_what_msg(ec, pos, context))
            , position_(pos)
            , context_(context)
            , errc_(ec)
    {}

    parsing_error(const parsing_error&) noexcept = default;
    parsing_error& operator=(const parsing_error&) noexcept = default;

    /// The byte index of the last valid character in the input file.
    ///
    /// @note For an input with n bytes, 1 is the index of the first character and
    ///       n+1 is the index of the terminating null byte or the end of file.
    ///
    std::size_t position() const noexcept { return position_; }

    /// Retu
    std::optional<bencode_type> context() const { return context_; }

    parsing_errc errc() const noexcept
    { return errc_; }

private:
    parsing_error(const char* what, parsing_errc ec, std::size_t position, std::optional<bencode_type> context)
            : exception(what)
            , position_(position)
            , context_(context)
            , errc_(ec)
    { };

    static std::string make_what_msg(parsing_errc ec, std::size_t pos, std::optional<bencode_type> context)
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
    parsing_errc errc_;
};


} // namespace bencode
