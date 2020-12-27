#pragma once
#include <stack>
#include <optional>
#include <string_view>
#include <type_traits>
#include <iostream>

#include <gsl-lite/gsl-lite.hpp>

#include "bencode/detail/utils.hpp"
#include "bencode/detail/concepts.hpp"
#include "bencode/detail/encoding_error.hpp"
#include "bencode/detail/events/encode_to.hpp"
#include "bencode/detail/events/events.hpp"
#include "bencode/detail/encoding_error.hpp"

namespace bencode {

namespace detail {

struct event_connector_tag {};

struct begin_list_tag : event_connector_tag {};
struct end_list_tag   : event_connector_tag {};
struct begin_dict_tag : event_connector_tag {};
struct end_dict_tag   : event_connector_tag {};

}

inline constexpr auto begin_list = detail::begin_list_tag {};
inline constexpr auto end_list   = detail::end_list_tag   {};
inline constexpr auto begin_dict = detail::begin_dict_tag {};
inline constexpr auto end_dict   = detail::end_dict_tag   {};


template <typename T>
concept event_connector_tag = std::is_base_of_v<detail::event_connector_tag, T>;


enum class connector_state
{
    expect_list_value,
    expect_dict_key,
    expect_dict_value,
};


/// Connect events from multiple producer to an event consumer using a stream interface.
template <event_consumer Consumer>
class event_connector
{
public:
    explicit event_connector(Consumer& consumer)
            : consumer_(consumer)
    {}

    template <event_connector_tag T>
    event_connector& operator<<(T tag) {
        update_state(tag);
        if (error_.has_value()) throw encoding_error(*error_);
        return *this;
    }

    template <typename U, typename T = std::remove_cvref_t<U>>
    /// \cond CONCEPTS
    requires event_producer<T> && (!event_connector_tag<T>)
    /// \endcond
    event_connector& operator<<(U&& value)
    {
        if (!stack_.empty()) {
            auto context = stack_.top();
            switch (context) {
            case connector_state::expect_dict_key: {
                handle_dict_key(std::forward<U>(value));
                break;
            }
            case connector_state::expect_dict_value: {
                handle_dict_value(std::forward<U>(value));
                break;
            }
            case connector_state::expect_list_value: {
                handle_list_value(std::forward<U>(value));
                break;
            }
            }
        } else {
            connect(consumer_, std::forward<U>(value));
        }
        if (error_.has_value()) throw encoding_error(*error_);
        return *this;
    }

private:
    void update_state(detail::begin_list_tag)
    {
        if (!stack_.empty() && stack_.top() == connector_state::expect_dict_key) [[unlikely]] {
            error_ = encoding_errc::invalid_dict_key;
            return;
        }
        stack_.push(connector_state::expect_list_value);
        consumer_.begin_list();
    }

    void update_state(detail::begin_dict_tag)
    {
        if (!stack_.empty() && stack_.top() == connector_state::expect_dict_key) [[unlikely]] {
            error_ = encoding_errc::invalid_dict_key;
            return;
        }
        stack_.push(connector_state::expect_dict_key);
        consumer_.begin_dict();
    }

    void update_state(detail::end_list_tag)
    {
        if (stack_.empty() || stack_.top() != connector_state::expect_list_value) [[unlikely]] {
            error_ = encoding_errc::unexpected_end_list;
            return;
        }

        stack_.pop();
        // check if by finishing a list we completed a dict bvalue
        if (!stack_.empty() && stack_.top() == connector_state::expect_dict_value) {
            stack_.top() = connector_state::expect_dict_key;
        }
        consumer_.end_list();
    }

    void update_state(detail::end_dict_tag)
    {
        if (stack_.empty() || stack_.top() != connector_state::expect_dict_key) [[unlikely]] {
            error_ = encoding_errc::unexpected_end_dict;
            return;
        }

        stack_.pop();
        // check if by finishing a dict we completed a dict bvalue.
        if (!stack_.empty() && stack_.top() == connector_state::expect_dict_value) {
            stack_.top() = connector_state::expect_dict_key;
        }
        consumer_.end_dict();
    }

    template <typename U, typename T = std::remove_cvref_t<U>>
    requires event_producer<T>
    inline void handle_dict_key(U&& key)
    {
        Expects(!stack_.empty());
        Expects(stack_.top() == connector_state::expect_dict_key);

        if constexpr (!serializable_to<T, bencode_type::string>) [[unlikely]] {
            error_ = encoding_errc::invalid_dict_key;
            return;
        }

        stack_.top() = connector_state::expect_dict_value;
        connect(consumer_, std::forward<U>(key));
        consumer_.dict_key();
    }

    template <typename U, typename T = std::remove_cvref_t<U>>
    requires event_producer<T>
    inline void handle_dict_value(U&& value)
    {
        Expects(!stack_.empty());
        Expects(stack_.top() == connector_state::expect_dict_value);

        stack_.top() = connector_state::expect_dict_key;
        connect(consumer_, std::forward<U>(value));
        consumer_.dict_value();
    }

    template <typename U, typename T = std::remove_cvref_t<U>>
    requires event_producer<T>
    inline void handle_list_value(U&& value)
    {
        Expects(!stack_.empty());
        Expects(stack_.top() == connector_state::expect_list_value);

        connect(consumer_, std::forward<U>(value));
        consumer_.list_item();
    }

    Consumer& consumer_;
    std::stack<connector_state> stack_ {};
    std::optional<encoding_errc> error_ {};
};

template <event_consumer Consumer>
event_connector(Consumer& os) -> event_connector<Consumer>;

} // namespace bencode
