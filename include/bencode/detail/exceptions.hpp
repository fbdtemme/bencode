#pragma once
#include <string>
#include <exception>
#include <stdexcept>
#include <string_view>

namespace bencode {

using namespace std::string_literals;

/// Error code enumeration for type conversion errors.
enum class conversion_errc : std::uint8_t
{
    /// The active alternative is not an integral type.
    not_integer_type,
    /// The active alternative is not a string type.
    not_string_type,
    /// The active alternative is not a list type.
    not_list_type,
    /// The active alternative is not a dict type.
    not_dict_type,
    /// Conversion to a fixed size type and the size of the active alternative does not match.
    size_mismatch,
    /// Unspecified exception was thrown during construction of the requested type.
    construction_error,
    /// The mapped type of a dict cannot be converted to the requested type.
    dict_mapped_type_construction_error,
    /// The bvalue type of a list cannot be converted to the requested type.
    list_value_type_construction_error,
    /// The conversion to the given type is not defined.
    undefined_conversion
};

/// Returns a description of the `ec` bvalue.
constexpr std::string_view to_string_view(const conversion_errc& ec)
{
    switch (ec) {
    case conversion_errc::not_integer_type:
        return "bvalue not of integer type";
    case conversion_errc::not_string_type:
        return "bvalue not of string type";
    case conversion_errc::not_list_type:
        return "bvalue not of list type";
    case conversion_errc::not_dict_type:
        return "bvalue not of dict type";
    case conversion_errc::size_mismatch:
        return "size mismatch between bvalue and destination type";
    case conversion_errc::list_value_type_construction_error:
        return "exception thrown during construction of list bvalue type.";
    case conversion_errc::dict_mapped_type_construction_error:
        return "exception thrown during construction of dict mapped type.";
    case conversion_errc::construction_error:
        return "exception thrown during construction of destination type.";
    default:
        return "unknown error";
    }
}


/// Error thrown when trying to convert a bvalue or bview to a type that does
/// not match the type of the current alternative.
class conversion_error : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;

    explicit conversion_error(const conversion_errc& ec)
        : runtime_error(std::string(to_string_view(ec)))
        , errc_(ec) {};

    conversion_errc error_code() const noexcept { return errc_; }

    conversion_errc errc_;
};

/// Base class for bad_bvalue_access and bad_bview_access exceptions.
class bad_access : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

} // namespace bencode
