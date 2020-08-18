#pragma once

#include <cstdint>
#include <vector>
#include <stack>
#include <algorithm>
#include <compare>
#include <gsl/gsl_assert>

#include "bencode/detail/bencode_type.hpp"
#include "bencode/detail/symbol.hpp"
#include "bencode/detail/utils.hpp"

#include "bencode/detail/parser/common.hpp"
#include "bencode/detail/parser/base_parser.hpp"
#include "bencode/detail/parser/error.hpp"
#include "bencode/detail/bitmask_operators.hpp"

namespace bencode {
enum class descriptor_type : std::uint8_t;
BENCODE_ENABLE_BITMASK_OPERATORS(descriptor_type);

}
namespace bencode {

/// Enumeration type specifying the data stored in a descriptor.
enum class descriptor_type : std::uint8_t
{
    // core types
    integer = 0x01,     //< descriptor hold integer data
    string = 0x02,      //< descriptor hold string data
    list = 0x04,        //< descriptor hold list data
    dict = 0x08,        //< descriptor hold dict data
    // modifiers
    none = 0x00,         //< object has no modifiers
    list_value = 0x10,   //< object is a list item
    dict_key = 0x20,     //< string is a dictionary key
    dict_value = 0x40,   //< object is a dictionary bvalue
    end = 0x80,          //< end token for a list or dict
    stop = 0xF0          //< flag for very last token of buffer
};

/// Class describing the bvalue and metadata contained in bencoded tokens.
//
struct descriptor
{
private:
    /// the decoded bvalue of an bencoded integer
    struct integer_data
    {
        std::int64_t value;
    };

    /// Data to describes a bencoded data types.
    /// The meaning of the fields varies with the `descriptor_type`.
    ///     * string:
    ///         offset: offset to the first character byte of the string.
    ///         size:   length of the string.
    ///     * list
    ///         offset: number of tape_entry objects between the matching list start/end token.
    ///         size:   the number of bencoded values in list
    ///     * dict
    ///         offset: number of tape_entry objects between the matching list start/end token.
    ///         size:   the number of bencoded key bvalue pairs in the list
    ///
    struct structured_data
    {
        std::uint32_t offset;
        std::uint32_t size;
    };

    union descriptor_data
    {
        integer_data integer;
        structured_data structured;
    };

public:
    template <typename... Args>
    constexpr descriptor(descriptor_type type, std::size_t position, Args... args) noexcept
            : type_(type)
            , position_(position)
            , data_()
    {
        if constexpr(sizeof...(Args) == 0) {}
        else if constexpr(sizeof...(Args) == 1) {
            if (is_integer())
                data_.integer = {args...};
        }
        else if constexpr (sizeof...(Args) == 2) {
            if (is_string() || is_list() || is_dict())
                data_.structured = {static_cast<std::uint32_t>(args)...};
        }
        else {
            static_assert((detail::always_false_v<Args> || ...), "invalid number of arguments");
        }
    }

    [[nodiscard]]
    constexpr auto type() const noexcept -> bencode_type
    {
        if (is_integer()) return bencode_type::integer;
        if (is_string())  return bencode_type::string;
        if (is_list())    return bencode_type::list;
        if (is_dict())    return bencode_type::dict;
        BENCODE_UNREACHABLE;
    }
    /// Returns true if the descriptor describes a bencoded integer, false otherwise.
    constexpr bool is_integer() const noexcept
    { return (type_ & descriptor_type::integer) == descriptor_type::integer; }

    /// Returns true if the descriptor describes a bencoded string, false otherwise.
    constexpr bool is_string() const noexcept
    { return (type_ & descriptor_type::string) == descriptor_type::string; }

    /// Returns true if the descriptor describes a bencoded list start token.
    constexpr bool is_list_begin() const noexcept
    {
        constexpr auto mask = (descriptor_type::list | descriptor_type::end);
        return (type_ & mask) == descriptor_type::list;
    }

    /// Returns true if the descriptor describes the last element of a bencoded list,
    /// false otherwise.
    constexpr bool is_list_end() const noexcept
    {
        constexpr auto mask = (descriptor_type::list | descriptor_type::end);
        return (type_ & mask) == mask;
    }

    constexpr bool is_dict_begin() const noexcept
    {
        constexpr auto mask = (descriptor_type::dict | descriptor_type::end);
        return (type_ & mask) == descriptor_type::dict;
    }

    constexpr bool is_dict_end() const noexcept
    {
        constexpr auto mask = (descriptor_type::dict | descriptor_type::end);
        return (type_ & mask) == mask;
    }

    constexpr bool is_list() const noexcept
    { return (type_ & descriptor_type::list) == descriptor_type::list; }

    constexpr bool is_dict() const noexcept
    { return (type_ & descriptor_type::dict) == descriptor_type::dict; }

    constexpr bool is_list_value() const noexcept
    {
        return (type_ & descriptor_type::list_value) == descriptor_type::list_value;
    }

    constexpr bool is_dict_key() const noexcept
    { return (type_ & descriptor_type::dict_key) == descriptor_type::dict_key; }

    constexpr bool is_dict_value() const noexcept
    { return (type_ & descriptor_type::dict_value) == descriptor_type::dict_value; }

    constexpr auto position() const noexcept -> std::uint32_t
    { return position_; }

    constexpr auto value() const noexcept -> std::int64_t
    {
        Expects(is_integer());
        return data_.integer.value;
    }

    constexpr void set_value(std::int64_t v) noexcept
    {
        Expects(is_integer());
        data_.integer.value = v;
    };

    // size of a string / list or map
    constexpr auto size() const noexcept -> std::uint32_t
    {
        Expects(is_string() || is_list_begin() || is_dict_begin());
        return data_.structured.size;
    }

    constexpr void set_size(std::uint32_t v) noexcept
    {
        Expects(is_string() || is_list() || is_dict());
        data_.structured.size = v;
    }

    /// offset to matching begin/end token for list/dict, counted from the first value
    /// token of the list/dict
    /// offset to start of actual string bvalue for strings
    constexpr auto offset() const noexcept -> std::uint32_t
    {
        Expects(is_string() || is_list() || is_dict());
        return data_.structured.offset;
    }

    constexpr void set_offset(std::uint32_t v) noexcept
    {
        Expects(is_string() || is_list() || is_dict());
        data_.structured.offset = v;
    }

    constexpr void set_stop_flag(bool flag = true) noexcept
    { type_ |= descriptor_type::stop; }

    constexpr bool operator==(const descriptor& that) const noexcept
    {
        if (type_ != that.type_) return false;
        if (position_ != that.position_) return false;
        if (is_integer()) {
            return value() == that.value();
        } else {
            return (offset() == that.offset()) && (size() == that.size());
        }
    }

protected:
    descriptor_type type_;
    std::uint32_t position_;
    descriptor_data data_;
};

}