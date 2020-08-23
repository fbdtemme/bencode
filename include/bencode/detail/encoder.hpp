#pragma once
#include <stack>
#include <optional>
#include <string_view>
#include <type_traits>
#include <gsl/gsl>
#include <iostream>

#include <bencode/detail/utils.hpp>
#include <bencode/detail/concepts.hpp>
#include <bencode/detail/encoding_error.hpp>
#include <bencode/detail/events/encode_to.hpp>
#include <bencode/detail/events/events.hpp>
#include "encoding_error.hpp"

namespace bencode {

namespace detail {

struct encoding_ostream_tag_type {};

struct begin_list_tag : encoding_ostream_tag_type {};
struct end_list_tag   : encoding_ostream_tag_type {};
struct begin_dict_tag : encoding_ostream_tag_type {};
struct end_dict_tag   : encoding_ostream_tag_type {};

}

inline constexpr auto begin_list = detail::begin_list_tag {};
inline constexpr auto end_list   = detail::end_list_tag   {};
inline constexpr auto begin_dict = detail::begin_dict_tag {};
inline constexpr auto end_dict   = detail::end_dict_tag   {};


template <typename T>
concept encoding_ostream_tag = std::is_base_of_v<detail::encoding_ostream_tag_type, T>;


enum class encoder_state
{
    expect_list_value,
    expect_dict_key,
    expect_dict_value,
};


/// Encode input to the underlying storage ostream or output_iterator.
/// Data is passed using the ostream interface,
template <event_consumer Consumer>
class encoder
{
public:
    explicit encoder(std::ostream& os)
        : consumer_(events::encode_to(std::ostreambuf_iterator<char>(os)))
    {}

    template <std::output_iterator<char> OIter>
    explicit encoder(OIter it)
        : consumer_(events::encode_to(it))
    {}

    template <encoding_ostream_tag T>
    encoder& operator<<(T tag) {
        update_state(tag);
        if (error_.has_value()) throw encoding_error(*error_);
        return *this;
    }

    template <typename U, typename T = std::remove_cvref_t<U>>
    /// \cond CONCEPTS
        requires event_producer<T> && (!encoding_ostream_tag<T>)
    /// \endcond
    encoder& operator<<(U&& value)
    {
        if (!stack_.empty()) {
            auto context = stack_.top();
            switch (context) {
                case encoder_state::expect_dict_key: {
                    handle_dict_key(std::forward<U>(value));
                    break;
                }
                case encoder_state::expect_dict_value: {
                    handle_dict_value(std::forward<U>(value));
                    break;
                }
                case encoder_state::expect_list_value: {
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
        if (!stack_.empty() && stack_.top() == encoder_state::expect_dict_key) [[unlikely]] {
            error_ = encoding_errc::invalid_dict_key;
        }
        stack_.push(encoder_state::expect_list_value);
        consumer_.begin_list();
    }

    void update_state(detail::begin_dict_tag)
    {
        if (!stack_.empty() && stack_.top() == encoder_state::expect_dict_key) [[unlikely]] {
            error_ = encoding_errc::invalid_dict_key;
        }
        stack_.push(encoder_state::expect_dict_key);
        consumer_.begin_dict();
    }

    void update_state(detail::end_list_tag)
    {
        if (stack_.empty() || stack_.top() != encoder_state::expect_list_value) [[unlikely]] {
            error_ = encoding_errc::unexpected_end_list;
        }

        stack_.pop();
        // check if by finishing a list we completed a dict bvalue
        if (!stack_.empty() && stack_.top() == encoder_state::expect_dict_value) {
            stack_.top() = encoder_state::expect_dict_key;
        }
        consumer_.end_list();
    }

    void update_state(detail::end_dict_tag)
    {
        if (stack_.empty() || stack_.top() != encoder_state::expect_dict_key) [[unlikely]] {
            error_ = encoding_errc::unexpected_end_dict;
        }

        stack_.pop();
        // check if by finishing a dict we completed a dict bvalue.
        if (!stack_.empty() && stack_.top() == encoder_state::expect_dict_value) {
            stack_.top() = encoder_state::expect_dict_key;
        }
        consumer_.end_dict();
    }

    template <typename U, typename T = std::remove_cvref_t<U>>
        requires event_producer<T>
    inline bool handle_dict_key(U&& key)
    {
        Expects(!stack_.empty());
        Expects(stack_.top() == encoder_state::expect_dict_key);

        if constexpr (!serializable_to<T, bencode_type::string>) [[unlikely]] {
            error_ = encoding_errc::invalid_dict_key;
            return false;
        }

        stack_.top() = encoder_state::expect_dict_value;
        connect(consumer_, std::forward<U>(key));
        consumer_.dict_key();
        return true;
    }

    template <typename U, typename T = std::remove_cvref_t<U>>
        requires event_producer<T>
    inline void handle_dict_value(U&& value)
    {
        Expects(!stack_.empty());
        Expects(stack_.top() == encoder_state::expect_dict_value);

        stack_.top() = encoder_state::expect_dict_key;
        connect(consumer_, std::forward<U>(value));
        consumer_.dict_value();
    }

    template <typename U, typename T = std::remove_cvref_t<U>>
        requires event_producer<T>
    inline void handle_list_value(U&& value)
    {
        Expects(!stack_.empty());
        Expects(stack_.top() == encoder_state::expect_list_value);

        connect(consumer_, std::forward<U>(value));
        consumer_.list_item();
    }

    Consumer consumer_;
    std::stack<encoder_state> stack_ {};
    std::optional<encoding_errc> error_ {};
};

encoder(std::basic_ostream<char>& os)
    -> encoder<events::encode_to<std::ostreambuf_iterator<char>>>;

encoder(std::basic_ostringstream<char>& os)
    -> encoder<events::encode_to<std::ostreambuf_iterator<char>>>;

template <std::output_iterator<char> OIter>
encoder(OIter it) -> encoder<events::encode_to<OIter>>;


} // namespace bencode



#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
