#pragma once
#include <optional>
#include <system_error>
#include <stack>
#include <istream>

#include "bencode/detail/events/concepts.hpp"
#include "bencode/detail/symbol.hpp"
#include "bencode/detail/utils.hpp"
#include "bencode/detail/bencode_type.hpp"
#include "bencode/detail/parser/parser_options.hpp"
#include "bencode/detail/parser/parser_state.hpp"
#include "bencode/detail/parser/parsing_error.hpp"
#include <bencode/detail/parser/from_iters.hpp>

#if defined(BENCODE_ENABLE_SWAR)
#define BENCODE_FROM_CHARS_IMPL swar
#else
#define BENCODE_FROM_CHARS_IMPL serial
#endif

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

    /// Parse the input range and pass generated events to the event consumer.
    /// @returns true if successful, false if an error occured.
    template <typename R, event_consumer EC>
        requires std::convertible_to<rng::range_value_t<R>, char>
    bool parse(EC& consumer, const R& range) noexcept
    {
        begin_ = rng::begin(range);
        it_ = rng::begin(range);
        end_ = rng::end(range);
        error_ = std::nullopt;

        auto result = parse_loop(consumer);
        return result;
    }

    /// Parse a string_view and pass generated events to the event consumer.
    /// @returns true if successful, false if an error occured.
    template <event_consumer EC>
    bool parse(EC& consumer, std::string_view s) noexcept
    { return parse<std::string_view>(consumer, s); }

    bool has_error() noexcept
    { return error_.has_value(); }

    parsing_error error() noexcept
    {
        Expects(error_.has_value());
        return *error_;
    }

private:
    template <event_consumer EC>
    bool parse_loop(EC& consumer)
    {
        // aliases for brevity
        constexpr auto list_t = state::expect_list_value;
        constexpr auto dict_t = state::expect_dict_value;
        constexpr auto value_t = state::expect_value;

        while (it_ != end_ && !error_.has_value() ) {
            // verify bvalue limits
            if (value_count_ > options_.value_limit) [[unlikely]] {
                set_error(parsing_errc::value_limit_exceeded);
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
                        set_error(parsing_errc::expected_dict_key_or_end, btype::dict);
                        continue;
                    }
                }
                case state::expect_dict_value:
                {
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
                    set_error(parsing_errc::internal_error);
                }
                break;
            }
            // No current parsing context.
            // This means we are parsing the first element from the data
            // or we are parsing successive elements from a stream.
            handle_value<value_t>(consumer);
        }

        // Error exit path -> pass error to consumer
        if (has_error()) [[unlikely]] {
            consumer.error(*error_);
            return false;
        }
        if (!stack_.empty()) [[unlikely]] {
            if (stack_.top() == detail::parser_state::expect_dict_key) {
                set_error(parsing_errc::expected_dict_key_or_end);
            }
            if (stack_.top() == detail::parser_state::expect_list_value) {
                set_error(parsing_errc::expected_list_value_or_end);
            }
            return false;
        }

        Ensures(stack_.empty());
        Ensures(it_ == end_);
        return true;
    }

    template <bencode::event_consumer Consumer>
    bool handle_integer(Consumer& consumer)
    {
        Expects(*it_ == symbol::begin_integer);

        std::int64_t value;
        detail::from_iters_result<Iterator> result;

        if constexpr (std::convertible_to<Iterator, const char*>) {
            result = detail::binteger_from_iters(it_, end_, value,
                                                 detail::implementation::BENCODE_FROM_CHARS_IMPL);
        } else {
            result = detail::binteger_from_iters(it_, end_, value);
        }

        it_ = result.iter;

        if (result.ec != parsing_errc{}) [[unlikely]] {
            set_error(result.ec, btype::integer);
            return false;
        }

        consumer.integer(value);
        ++value_count_;
        return true;
    }

    template <bencode::event_consumer Consumer>
    bool handle_string(Consumer& consumer)
    {
        Expects(*it_ == symbol::digit);

        std::string value;
        const auto result = detail::bstring_from_iters(it_, end_, value);

        it_ = result.iter;

        if (result.ec != parsing_errc{}) [[unlikely]] {
            set_error(result.ec, btype::string);
            return false;
        }

        consumer.string(std::move(value));
        ++value_count_;
        return true;
    }

    template <state ParserState, bencode::event_consumer Consumer>
    bool handle_value(Consumer& consumer)
    {
        Expects(ParserState != state::expect_dict_key);
        Expects(stack_.empty() || stack_.top() == ParserState);

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
            if constexpr (ParserState == state::expect_list_value) {
                set_error(parsing_errc::expected_list_value_or_end, btype::list);
            }
            else if constexpr (ParserState == state::expect_dict_value) {
                set_error(parsing_errc::expected_dict_value, btype::dict);
            } else {
                set_error(parsing_errc::expected_value);
            }
            return false;
        }
        }
    }

    template <bencode::event_consumer Consumer>
    bool handle_list_begin(Consumer& consumer)
    {
        Expects(*it_ == symbol::begin_list);

        if (stack_.size() >= options_.recursion_limit) {
            set_error(parsing_errc::recursion_depth_exceeded);
            return false;
        }
        ++it_;
        stack_.push(state::expect_list_value);
        consumer.begin_list();
        ++value_count_;
        return true;
    }

    template <bencode::event_consumer Consumer>
    bool handle_dict_begin(Consumer& consumer)
    {
        Expects(*it_ == symbol::begin_dict);

        if (stack_.size() >= options_.recursion_limit) {
            set_error(parsing_errc::recursion_depth_exceeded);
            return false;
        }
        ++it_;
        stack_.push(state::expect_dict_key);
        consumer.begin_dict();
        ++value_count_;
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

        std::string value;
        auto result = detail::bstring_from_iters(it_, end_, value);

        it_ = result.iter;

        if (result.ec != parsing_errc{}) [[unlikely]] {
            set_error(result.ec, btype::string);
            return false;
        }

        stack_.top() = state::expect_dict_value;
        consumer.string(std::move(value));
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

    inline void set_error(parsing_errc ec,
                          std::optional<bencode_type> context = std::nullopt) noexcept
    {
        auto pos = std::distance(begin_, it_);
        error_.emplace(ec, pos, context);
    }

private:
    iterator_t begin_;
    iterator_t it_;
    sentinel_t end_;
    std::stack<state> stack_{};
    std::optional<parsing_error> error_;
    std::uint32_t value_count_ = 0;
    options options_;
};

} // namespace bencode