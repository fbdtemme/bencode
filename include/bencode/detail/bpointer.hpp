// TODO: Compile-time json pointer implementation for bencode:basic_bvalue.
//       Use LL(1) parser from https://github.com/hanickadot/compile-time-regular-expressions
//       Check talk at: https://www.youtube.com/watch?v=g51_HYn_CqE
#pragma once
#include <vector>
#include <string>
#include <string_view>
#include <stdexcept>
#include <algorithm>
#include <compare>
#include <gsl/gsl_assert>
#include <fmt/format.h>

#include "bencode/detail/parser/common.hpp"
#include "bencode/detail/concepts.hpp"
#include "bencode/detail/exceptions.hpp"


namespace bencode {

namespace rng = std::ranges;

class bpointer_error : public bencode_exception
{
public:
    using bencode_exception::bencode_exception;
};


class bpointer
{
public:
    explicit bpointer(std::string_view expression)
        : tokens_(split_tokens(expression))
    {}

    bool operator==(const bpointer& rhs) const noexcept
    {
        return this->tokens_ == rhs.tokens_;
    }

    std::strong_ordering operator<=>(const bpointer& rhs) const noexcept
    {
        return std::compare_strong_order_fallback(this->tokens_, rhs.tokens_);
    }

    bpointer& operator/=(const bpointer& ptr)
    {
        tokens_.insert(
                tokens_.end(),
                ptr.tokens_.begin(),
                ptr.tokens_.end());
        return *this;
    }

    bpointer& operator/=(std::string_view token)
    {
        push_back(std::string(token));
        return *this;
    }

    bpointer& operator/=(std::size_t array_idx)
    {
        return *this /= std::to_string(array_idx);
    }

    friend bpointer operator/(const bpointer& lhs, const bpointer& rhs)
    {
        return bpointer(lhs) /= rhs;
    }

    friend bpointer operator/(const bpointer& ptr, std::string_view token)
    {
        return bpointer(ptr) /= std::string(token);
    }

    friend bpointer operator/(const bpointer& ptr, std::size_t array_idx)
    {
        return bpointer(ptr) /= array_idx;
    }

    std::string to_string() const
    {
        std::string result {};
        rng::for_each(tokens_, [&](const auto& token) {
            result.push_back('/');
            result.append(escape(token));
        });
        return result;
    }

    explicit operator std::string() const
    { return to_string(); }

    /// Return the parent of this pointer
    bpointer parent() const
    {
        if (tokens_.empty()) {
            return *this;
        }

        bpointer res = *this;
        res.pop_back();
        return res;
    }

    bool empty() const noexcept
    {
        return tokens_.empty();
    }

    void push_back(const std::string& dict_key)
    {
        tokens_.push_back(dict_key);
    }

    void push_back(std::string&& dict_key)
    {
        tokens_.push_back(std::move(dict_key));
    }

    /// Remove last token.
    void pop_back()
    {
        tokens_.pop_back();
    }

    const std::string& front() const
    {
        return tokens_.front();
    }

    std::string& front()
    {
        return tokens_.front();
    }

    const std::string& back() const
    {
        return tokens_.back();
    }

    std::string& back()
    {
        return tokens_.back();
    }

    template <bvalue_or_bview T>
    decltype(auto) get(T&& bv) const
    {
        auto* ptr = &bv;

        for (const auto& token : tokens_) {
            switch (ptr->type()) {
            case bencode_type::dict: {
                ptr = &(get_dict(*ptr).at(token));
                break;
            }
            case bencode_type::list: {
                if (token == "-") [[unlikely]]
                    throw std::out_of_range("invalid list index: '-'");

                auto res = detail::parse_integer<std::uint64_t>(token.begin(), token.end());
                if (!res) [[unlikely]]
                        throw bpointer_error(fmt::format("unresolved token '{}'", token));

                ptr = &(get_list(*ptr).at(res.value()));
                break;
            }
            default:
                throw bpointer_error(
                        fmt::format("unresolved token '{}'", token));
            }
        }
        return detail::forward_like<T>(*ptr);
    }


    template <bvalue_or_bview T>
    const T* get_if(const T* bv) const
    {
        auto* ptr = &bv;

        for (const auto& token : tokens_) {
            switch (ptr->type()) {

            case bencode_type::dict: {
                const auto* d = get_if_dict(ptr);

                if (!d) [[unlikely]]
                    return nullptr;

                if (!d->contains(token)) [[unlikely]]
                    return nullptr;

                ptr = &(d->operator[](token));
                break;
            }

            case bencode_type::list: {
                if (token == "-") [[unlikely]]
                    return nullptr;

                auto res = detail::parse_integer<std::uint64_t>(token.begin(), token.end());

                if (!res)
                    return nullptr;

                const auto* l = get_if_list(ptr);
                if (!l)
                    return nullptr;

                if (res.value() >= l->size())
                    return nullptr;

                ptr = &(l->operator[](res.value()));
                break;
            }
            default:
                throw bpointer_error(
                        fmt::format("unresolved token '{}'", token));
            }
        }
        return detail::forward_like<T>(*ptr);
    }

private:
    template <rng::input_range Rng>
        requires std::same_as<rng::range_value_t<Rng>, char>
    static std::string escape(Rng&& token)
    {
        auto result = std::string(token);
        auto prev = rng::begin(token);
        auto last = rng::end(token);
        auto it = rng::find_first_of(rng::subrange(prev, last), "~/");
        for ( ; it != last ; prev = ++it, it = rng::find_first_of(rng::subrange(prev, last), "~/"))
        {
            rng::copy(prev, it, std::back_inserter(result));
            auto c = *it;
            if (c == '~')
                result.append("~0");
            else if (c == '/')
                result.append("~1");
            else
                Ensures(false);
        }
        return result;
    }

    template <rng::input_range Rng>
        requires std::same_as<rng::range_value_t<Rng>, char>
    static std::string unescape(Rng&& token)
    {
        std::string result {};

        auto end = rng::end(token);
        auto prev = rng::begin(token);
        auto cur = rng::find(prev, end, '~');
        for ( ; cur != rng::end(token); prev = cur, cur = rng::find(prev, end, '~'))
        {
            // copy all characters up until the next escape character
            rng::copy(prev, cur, std::back_inserter(result));

            auto next_char = *++cur;
            if (next_char == '0')
                result.push_back('~');
            else if (next_char == '1')
                result.push_back('/');
            else
                throw bpointer_error("escape character '~' must be followed with '0' or '1'");
        }
        // copy all characters from last ~ to end
        rng::copy(prev, rng::end(token), std::back_inserter(result));
        return result;
    }

    template <rng::input_range Rng>
        requires std::same_as<rng::range_value_t<Rng>, char>
    std::vector<std::string> split_tokens(Rng&& reference)
    {
        std::vector<std::string> result {};

        if (rng::empty(reference)) {
            return result;
        }

        if (*rng::begin(reference) != '/') [[unlikely]] {
            throw bencode::bpointer_error("bencode pointer must be empty or begin with '/'");
        }

        auto prev = std::next(rng::begin(reference));
        auto last = rng::end(reference);
        auto cur = std::find(prev, last, '/');
        for ( ; cur != last; prev = std::next(cur), cur = std::find(prev, last, '/'))
        {
           auto token = std::string(prev, cur);
           token = unescape(token);
           result.push_back(std::move(token));
        }
        result.push_back(unescape(std::string(prev, last)));
        return result;
    }

    std::vector<std::string> tokens_;
};

}