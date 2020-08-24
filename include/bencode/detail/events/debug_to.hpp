#pragma once

#include <optional>
#include <string_view>
#include <ostream>
#include <concepts>

#include "fmt/format.h"

namespace bencode::events {

template <typename OIter>
    requires std::output_iterator<OIter, char>
class debug_to
{
public:
    explicit constexpr debug_to(OIter out) noexcept
            : out_(out) {}

    debug_to(const debug_to&) = delete;
    debug_to(debug_to&&) = delete;
    debug_to& operator=(const debug_to&) = delete;
    debug_to& operator=(debug_to&&) = delete;

    constexpr void integer(std::int64_t value) noexcept
    {
        fmt::format_to(out_, "integer ({})\n", value);
    }

    constexpr void string(std::string_view value) noexcept
    {
        fmt::format_to(out_, "string (size={}, value=\"{}\")\n", value.size(), value);
    }

    constexpr void begin_list([[maybe_unused]] std::optional<std::size_t> size = std::nullopt) noexcept
    {
        if (size) {
            fmt::format_to(out_, "begin list (size={})\n", *size);
        } else {
            fmt::format_to(out_, "begin list\n");
        }
    }

    constexpr void list_item() noexcept
    {
        fmt::format_to(out_, "list item\n");
    }

    constexpr void end_list([[maybe_unused]] std::optional<std::size_t> size = std::nullopt) noexcept
    {
        if (size) {
            fmt::format_to(out_, "end list (size={})\n", *size);
        } else {
            fmt::format_to(out_, "end list\n");
        }
    }

    constexpr void begin_dict([[maybe_unused]] std::optional<std::size_t> size = std::nullopt) noexcept
    {
        if (size) {
            fmt::format_to(out_, "begin dict (size={})\n", *size);
        } else {
            fmt::format_to(out_, "begin dict\n");
        }
    }

    constexpr void end_dict([[maybe_unused]] std::optional<std::size_t> size = std::nullopt) noexcept
    {
        if (size) {
            fmt::format_to(out_, "end dict (size={})\n", *size);
        } else {
            fmt::format_to(out_, "end dict\n");
        }
    }

    constexpr void dict_key() noexcept
    {
        fmt::format_to(out_, "dict key\n");
    };

    constexpr void dict_value() noexcept
    {
        fmt::format_to(out_, "dict value\n");
    };

    constexpr void error(const bencode::parsing_error& e)
    {
        fmt::format_to(out_, "error: {}", e.what());
    }

private:
    OIter out_;
};

template <typename OutputIterator>
debug_to(OutputIterator out) -> debug_to<OutputIterator>;

debug_to(std::basic_ostream<char>& os) -> debug_to<std::ostreambuf_iterator<char>>;

static_assert(event_consumer<debug_to<char*>>);


} // namespace bencode::events::consumer