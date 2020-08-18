#pragma once
#include <cassert>
#include <cstdint>
#include <vector>
#include <stack>
#include <algorithm>
#include <ranges>
#include <nonstd/expected.hpp>

#include "bencode/detail/bencode_type.hpp"
#include "bencode/detail/symbol.hpp"
#include "bencode/detail/bitmask_operators.hpp"
#include "bencode/detail/utils.hpp"

#include "bencode/detail/parser/common.hpp"
#include "bencode/detail/parser/base_parser.hpp"
#include "bencode/detail/parser/error.hpp"

#include "bencode/detail/descriptor.hpp"
#include "bencode/detail/descriptor_table.hpp"

namespace bencode {

namespace rng = std::ranges;

namespace detail {

struct descriptor_parser_stack_frame
{
    detail::parser_state state;
    std::uint32_t position;
    std::uint32_t size;
};

constexpr auto descriptor_type_modifier(parser_state s) noexcept -> descriptor_type
{
    switch (s) {
    case parser_state::expect_list_value:
        return descriptor_type::list_value;
    case parser_state::expect_dict_key:
        return descriptor_type::dict_key;
    case parser_state::expect_dict_value:
        return descriptor_type::dict_value;
    default:
        return {};
    }
};

}

/// Parse bencoded data into a descriptor_table.
template <typename Iterator = const char*, typename Sentinel = Iterator>
class descriptor_parser
{
    using state = detail::parser_state;
    using options = parser_options;
    using stack_frame = detail::descriptor_parser_stack_frame;

    using iterator_t = Iterator;
    using sentinel_t = Sentinel;

public:
    explicit descriptor_parser(parser_options options = {})
            : options_(options)
            , descriptors_()
    {}

    template <typename R>
    requires rng::contiguous_range<R> && rng::sized_range<R> &&
             std::convertible_to<rng::range_value_t<R>, char>
    auto parse(const R& range) -> nonstd::expected<descriptor_table, parse_error>
    {
        begin_ = rng::data(range);
        it_ = rng::data(range);
        end_ = std::next(rng::data(range), rng::size(range));
        descriptors_.clear();
        if (!stack_.empty()) {
            stack_ = {};
        }
        error_ = std::nullopt;

        auto success = parse_loop();

        if (!success) {
            Expects(error_);
            return nonstd::make_unexpected(*error_);
        }
        return descriptor_table(std::move(descriptors_), rng::data(range));
    }

    std::optional<parse_error> error() {
        return error_;
    }

private:
    auto parse_loop() noexcept -> bool
    {
        // aliases for brevity
        // TODO [c++20] : change to using enum
        constexpr auto list_t = state::expect_list_value;
        constexpr auto dict_t = state::expect_dict_value;
        constexpr auto value_t = state::expect_value;

        while (it_ != end_ && !error_.has_value()) {

            // verify bvalue limits
            if (descriptors_.size() > options_.value_limit) [[unlikely]] {
                set_error(parser_errc::value_limit_exceeded);
                return false;
            }

            // read a character
            const char c = *it_;

            // check current parsing context
            if (!stack_.empty()) {
                auto [context, position, size] = stack_.top();

                switch (context) {
                case state::expect_dict_key: {
                    if (c == symbol::digit) [[likely]] {
                        handle_dict_key();
                        continue;
                    }
                    else if (c == symbol::end) {
                        handle_dict_end();
                        continue;
                    }
                    else [[unlikely]] {
                        set_error(parser_errc::expected_dict_key_or_end);
                        continue;
                    }
                }
                case state::expect_dict_value: {
                    if (c == symbol::end) [[unlikely]] {
                        set_error(parser_errc::expected_dict_value);
                        continue;
                    }
                    handle_value<dict_t>();
                    continue;
                }
                case state::expect_list_value: {
                    if (c == symbol::end) [[unlikely]] {
                        handle_list_end();
                        continue;
                    }
                    else {
                        handle_value<list_t>();
                        continue;
                    }
                }
                default:
                    set_error(parser_errc::internal_error);
                    BENCODE_UNREACHABLE;
                }
            }
            // No current parsing context. This means we are parsing the first element
            // from the data or successive elements from a stream.
            handle_value<value_t>();
        }

        if (error_.has_value())
            return false;

        // set stop flag on last token
        if (!descriptors_.empty())
            descriptors_.back().set_stop_flag();

        Ensures(stack_.empty());
        Ensures(it_ == end_);
        return true;
    }

    inline auto handle_integer(descriptor_type modifier) noexcept -> bool
    {
        Expects(*it_ == symbol::begin_integer);

        const auto type = (descriptor_type::integer | modifier);
        auto& t = descriptors_.emplace_back(type, current_position());

        auto result = detail::bdecode_integer(it_, end_);

        if (!result) [[unlikely]] {
            set_error(result.error());
            return false;
        }
        t.set_value(*result);
        return true;
    }

    inline auto handle_string(descriptor_type modifier) noexcept -> bool
    {
        Expects(*it_ == symbol::digit);

        const auto type = (descriptor_type::string | modifier);
        const auto position = static_cast<std::size_t>(std::distance(begin_, it_));

        auto result = detail::bdecode_string_token(it_, end_);

        if (!result) [[unlikely]] {
            set_error(result.error());
            return false;
        }
        auto& t = descriptors_.emplace_back(type, position);
        t.set_offset(result->offset);
        t.set_size(result->size);
        return true;
    }

    template <state ParserState>
    inline auto handle_value() noexcept -> bool
    {
        Expects(ParserState != state::expect_dict_key);
        Expects(*it_ == symbol::value);

        constexpr auto type_modifier = detail::descriptor_type_modifier(ParserState);

        auto dispatch = [&](bool success = true) {
            if (!success) [[unlikely]] return false;

            if constexpr (ParserState == state::expect_dict_value) {
                auto& [st, start, size] = stack_.top();
                st = state::expect_dict_key;
                ++size;
            }
            else if constexpr (ParserState == state::expect_list_value) {
                auto& [st, start, size] = stack_.top();
                ++size;
            }
            return true;
        };

        const char c = *it_;

        switch (c) {
        case symbol::begin_integer:
            return dispatch(handle_integer(type_modifier));
        case symbol::begin_list:
            return handle_list_begin(type_modifier);
        case symbol::begin_dict:
            return handle_dict_begin(type_modifier);
        default: {
            if (c == symbol::digit) [[likely]] {
                return dispatch(handle_string(type_modifier));
            }
            set_error(parser_errc::expected_value);
            return false;
        }
        }
    }

    inline auto handle_list_begin(descriptor_type modifier) noexcept -> bool
    {
        Expects(*it_ == symbol::begin_list);

        const auto type = (descriptor_type::list | modifier);
        const auto position = current_position();

        if (stack_.size() == options_.recursion_limit) {
            set_error(parser_errc::recursion_depth_exceeded);
            return false;
        }

        descriptors_.emplace_back(type, position);
        stack_.push({
                .state = state::expect_list_value,
                .position = static_cast<std::uint32_t>(descriptors_.size()-1),
                .size = 0
        });

        ++it_;
        return true;
    }

    inline auto handle_dict_begin(descriptor_type modifier) -> bool
    {
        Expects(*it_ == symbol::begin_dict);

        const auto type = (descriptor_type::dict | modifier);
        const auto position = current_position();

        if (stack_.size() == options_.recursion_limit) {
            set_error(parser_errc::recursion_depth_exceeded);
            return false;
        }

        descriptors_.emplace_back(type, position);
        stack_.push({
                .state = state::expect_dict_key,
                .position = static_cast<std::uint32_t>(descriptors_.size()-1),
                .size = 0
        });

        ++it_;
        return true;
    }

    inline auto handle_list_end() -> bool
    {
        Expects(*it_ == symbol::end);
        Expects(stack_.top().state == state::expect_list_value);

        auto type = (descriptor_type::list | descriptor_type::end);
        const auto[state, start_pos, size] = stack_.top();
        const auto offset = descriptors_.size() - start_pos;
        const auto position = current_position();

        stack_.pop();
        ++it_;

        if (auto s = handle_nested_structures(); s)
            type |= detail::descriptor_type_modifier(*s);

        auto& t = descriptors_.emplace_back(type, position);
        descriptors_[start_pos].set_offset(offset);
        descriptors_[start_pos].set_size(size);
        t.set_offset(offset);
        t.set_size(size);
        return true;
    }

    inline auto handle_dict_end() noexcept -> bool
    {
        Expects(*it_ == symbol::end);
        Expects(stack_.top().state == state::expect_dict_key);

        auto type = (descriptor_type::dict | descriptor_type::end);
        const auto& [state, start_pos, size] = stack_.top();
        const auto offset = descriptors_.size() - start_pos;
        const auto position = current_position();

        stack_.pop();
        ++it_;
        if (auto s = handle_nested_structures(); s)
            type |= detail::descriptor_type_modifier(*s);

        auto& t = descriptors_.emplace_back(type, position);
        descriptors_[start_pos].set_offset(offset);
        descriptors_[start_pos].set_size(size);
        t.set_offset(offset);
        t.set_size(size);
        return true;
    }

    inline auto handle_dict_key() noexcept -> bool
    {
        Expects(stack_.top().state == state::expect_dict_key);
        Expects(*it_ == symbol::digit);

        constexpr auto type = (
                descriptor_type::string | descriptor_type::dict_key );
        auto position = current_position();

        auto result = detail::bdecode_string_token(it_, end_);

        if (!result) [[unlikely]] {
            set_error(result.error());
            return false;
        }

        stack_.top().state = state::expect_dict_value;

        auto& t = descriptors_.emplace_back(type, position);
        t.set_offset(result->offset);
        t.set_size(result->size);
        return true;
    }

    inline auto handle_nested_structures() noexcept -> std::optional<state>
    {
        if (stack_.empty()) return std::nullopt;

        auto&[state, position, size] = stack_.top();
        const auto old_state = state;
        ++size;

        switch (state) {
        case state::expect_dict_value:  state = state::expect_dict_key;   break;
        case state::expect_dict_key  :  state = state::expect_dict_value; break;
        }
        return old_state;
    }

    inline auto current_position() noexcept -> std::size_t
    { return (it_ - begin_); }

    inline void set_error(parser_errc ec) noexcept
    {
        error_.emplace(ec, current_position());
    }

    iterator_t begin_;
    iterator_t it_;
    sentinel_t end_;
    std::vector<descriptor> descriptors_;
    std::stack<stack_frame> stack_{};
    std::optional<parse_error> error_;
    const options options_;
};

}
