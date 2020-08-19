#pragma once
#include <optional>
#include <system_error>
#include <stack>
#include <istream>

#include <nonstd/expected.hpp>

#include "bencode/detail/events/concepts.hpp"

#include "bencode/detail/symbol.hpp"
#include "bencode/detail/utils.hpp"
#include "bencode/detail/bencode_type.hpp"
#include "bencode/detail/parser/common.hpp"
#include "bencode/detail/parser/base_parser.hpp"


#include "error.hpp"

namespace bencode {

namespace rng = std::ranges;

template <typename Iterator = const char*, typename Sentinel = Iterator>
class push_parser
{
    using state      = detail::parser_state;
    using iterator_t = Iterator;
    using sentinel_t = Sentinel;

public:
    using options = parser_options;

    explicit push_parser(const push_parser::options& options = {})
            : options_(options)
    {}

    template <typename R, event_consumer EC>
        requires std::convertible_to<rng::range_value_t<R>, char>
    bool parse(const R& range, EC& consumer) noexcept
    {
        begin_ = rng::begin(range);
        it_ = rng::begin(range);
        end_ = rng::end(range);

        if (!stack_.empty()) {
            stack_ = {};
        }
        error_ = std::nullopt;

        // aliases for brevity
        constexpr auto list_t = state::expect_list_value;
        constexpr auto dict_t = state::expect_dict_value;
        constexpr auto value_t = state::expect_value;

        while (it_ != end_) {
            if (error_.has_value()) [[unlikely]]
                return false;

            // verify bvalue limits
            if (value_count_ > options_.value_limit) [[unlikely]] {
                set_error(parser_errc::value_limit_exceeded);
                return false;
            }

            // read a character
            const char c = *it_;

            // check current parsing context
            if (!stack_.empty()) {
                state context = stack_.top();

                switch (context) {
                case state::expect_dict_key:
                {
                    if (c == symbol::digit) [[likely]] {
                        handle_dict_key(consumer);
                        continue;
                    }
                    else if (c == symbol::end) {
                        handle_dict_end(consumer);
                        continue;
                    }
                    else [[unlikely]] {
                        set_error(parser_errc::expected_dict_key_or_end);
                        continue;
                    }
                }
                case state::expect_dict_value:
                {
                    if (c == symbol::end) [[unlikely]] {
                        set_error(parser_errc::expected_dict_value);
                        continue;
                    }
                    handle_value<dict_t>(consumer);
                    continue;
                }
                case state::expect_list_value:
                {
                    if (c == symbol::end) {
                        handle_list_end(consumer);
                        continue;
                    }
                    else {
                        handle_value<list_t>(consumer);
                        continue;
                    }
                }
                default:
                    // this should not happen
                    set_error(parser_errc::internal_error);
                }
            }
            // No current parsing context.
            // This means we are parsing the first element from the data
            // or we are parsing successive elements from a stream.
            handle_value<value_t>(consumer);
        }

        // Error exit path -> pass error to consumer
        if (has_error()) {
            consumer.error(*error_);
            return false;
        }
        else {
            Ensures(stack_.empty());
            Ensures(it_ == end_);
            return true;
        }
    }

    auto has_error() noexcept -> bool
    { return error_.has_value(); }

    auto error() noexcept -> parse_error
    {
        Expects(error_.has_value());
        return *error_;
    }

private:
    template <bencode::event_consumer Consumer>
    bool handle_integer(Consumer& consumer)
    {
        Expects(*it_ == symbol::begin_integer);

        auto value = detail::bdecode_integer<std::int64_t>(it_, end_);

        if (!value) [[unlikely]] {
            set_error(value.error());
            return false;
        }

        consumer.integer(*value);
        return true;
    }

    template <bencode::event_consumer Consumer>
    bool handle_string(Consumer& consumer)
    {
        Expects(*it_ == symbol::digit);

        auto value = detail::bdecode_string(it_, end_);
        if (!value) [[unlikely]] {
            set_error(value.error());
            return false;
        }
        consumer.string(*value);

        return true;
    }

    template <state ParserState, bencode::event_consumer Consumer>
    bool handle_value(Consumer& consumer)
    {
        Expects(ParserState != state::expect_dict_key);
        Expects(stack_.empty() || stack_.top() == ParserState);
        Expects(*it_ == symbol::value);

        // TODO: merge this lambda with handle_nested_structures
        const auto dispatch = [&](bool success) {
            if (!success) [[unlikely]] return false;

            if constexpr (ParserState == state::expect_dict_value) {
                consumer.dict_value();
                stack_.top() = state::expect_dict_key;
            }
            else if constexpr (ParserState == state::expect_list_value) {
                consumer.list_item();
            }
            return true;
        };

        const char c = *it_;

        switch (c) {
        case symbol::begin_integer:
            return dispatch(handle_integer(consumer));
        case symbol::begin_list:
            return handle_list_begin(consumer);
        case symbol::begin_dict:
            return handle_dict_begin(consumer);
        default: {
            if (c == symbol::digit) [[likely]] {
                return dispatch(handle_string(consumer));
            }
            set_error(parser_errc::expected_value);
            return false;
        }
        }
    }

    template <bencode::event_consumer Consumer>
    bool handle_list_begin(Consumer& consumer)
    {
        Expects(*it_ == symbol::begin_list);

        if (stack_.size() == options_.recursion_limit) {
            set_error(parser_errc::recursion_depth_exceeded);
            return false;
        }
        ++it_;
        stack_.push(state::expect_list_value);
        consumer.begin_list();
        return true;
    }

    template <bencode::event_consumer Consumer>
    bool handle_dict_begin(Consumer& consumer)
    {
        Expects(*it_ == symbol::begin_dict);

        if (stack_.size() == options_.recursion_limit) {
            set_error(parser_errc::recursion_depth_exceeded);
            return false;
        }
        ++it_;
        stack_.push(state::expect_dict_key);
        consumer.begin_dict();
        return true;
    }


    template <bencode::event_consumer Consumer>
    bool handle_list_end(Consumer& consumer)
    {
        Expects(*it_ == symbol::end);
        Expects(stack_.top() == state::expect_list_value);

        ++it_;
        stack_.pop();
        consumer.end_list();
        handle_nested_structures(consumer);
        return true;
    }

    template <bencode::event_consumer Consumer>
    bool handle_dict_end(Consumer& consumer)
    {
        Expects(*it_ == symbol::end);
        Expects(stack_.top() == state::expect_dict_key);

        ++it_;
        stack_.pop();
        consumer.end_dict();
        handle_nested_structures(consumer);
        return true;
    }

    template <bencode::event_consumer Consumer>
    bool handle_dict_key(Consumer& consumer)
    {
        Expects(stack_.top() == state::expect_dict_key);
        Expects(*it_ == symbol::digit);

        auto value = detail::bdecode_string<std::string>(it_, end_);

        if (!value) [[unlikely]] {
            set_error(value.error());
            return false;
        }
        stack_.top() = state::expect_dict_value;
        consumer.string(std::move(*value));
        consumer.dict_key();
        return true;
    }

    template <bencode::event_consumer Consumer>
    inline void handle_nested_structures(Consumer& consumer)
    {
        if (stack_.empty()) return;
        auto& state = stack_.top();

        if (state == state::expect_list_value) {
            consumer.list_item();
        }
        else if (state == state::expect_dict_value) {
            consumer.dict_value();
            state = state::expect_dict_key;
        }
    }

    inline void set_error(parser_errc ec) {
        error_.emplace(ec, std::distance(begin_, it_));
    }

private:
    iterator_t begin_;
    iterator_t it_;
    sentinel_t end_;
    std::stack<state> stack_{};
    std::optional<parse_error> error_;
    std::uint32_t value_count_ = 0;
    const options& options_;
};

// CTAD hints

template <typename Range>
explicit push_parser(const Range&, parser_options = {})
        -> push_parser<typename rng::iterator_t<const Range>,
                       typename rng::sentinel_t<const Range>>;


template <typename Iterator, typename Sentinel>
explicit push_parser(Iterator first, Sentinel last, parser_options = {})
        -> push_parser<Iterator, Sentinel>;

} // namespace bencode