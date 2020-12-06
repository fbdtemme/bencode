#ifndef INC_BENCODE_HPP
#define INC_BENCODE_HPP

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/variant.hpp>

// Try to use std::string_view, N4480's version, or fall back to Boost's.

#ifdef __has_include
#  if __has_include(<string_view>) && (__cplusplus >= 201703L || \
      (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L))
#    include <string_view>
#    define BENCODE_STRING_VIEW std::string_view
#  elif __has_include(<experimental/string_view>) && \
        !defined(BENCODE_NO_STDLIB_EXTS)
#    include <experimental/string_view>
#    define BENCODE_STRING_VIEW std::experimental::string_view
#  endif
#endif

#ifndef BENCODE_STRING_VIEW
#  ifdef BENCODE_USE_STDLIB_EXTS
#    include <experimental/string_view>
#    define BENCODE_STRING_VIEW std::experimental::string_view
#  else
#    include <boost/version.hpp>
#    if BOOST_VERSION >= 106100
#      include <boost/utility/string_view.hpp>
#      define BENCODE_STRING_VIEW boost::string_view
#    else
#      include <boost/utility/string_ref.hpp>
#      define BENCODE_STRING_VIEW boost::string_ref
#    endif
#  endif
#endif

namespace bencode {

using integer = long long;
using integer_view = integer;
using string = std::string;
using string_view = BENCODE_STRING_VIEW;

using data = boost::make_recursive_variant<
        integer,
        string,
        std::vector<boost::recursive_variant_>,
        std::map<string, boost::recursive_variant_>
>::type;

using data_view = boost::make_recursive_variant<
        integer_view,
        string_view,
        std::vector<boost::recursive_variant_>,
        std::map<string_view, boost::recursive_variant_>
>::type;

using list = std::vector<data>;
using dict = std::map<string, data>;

using list_view = std::vector<data_view>;
using dict_view = std::map<string_view, data_view>;

enum eof_behavior {
    check_eof,
    no_check_eof
};

template<typename T>
data decode(T &begin, T end);

namespace detail {
template<bool View, typename T>
typename std::conditional<View, data_view, data>::type
decode_data(T &begin, T end);

template<bool View, typename T>
typename std::conditional<View, integer_view, integer>::type
decode_int(T &begin, T end) {
    assert(*begin == 'i');
    ++begin;

    bool positive = true;
    if(*begin == '-') {
        positive = false;
        ++begin;
    }

    // TODO: handle overflow
    integer value = 0;
    for(; begin != end && std::isdigit(*begin); ++begin)
        value = value * 10 + *begin - '0';
    if(begin == end)
        throw std::invalid_argument("unexpected end of string");
    if(*begin != 'e')
        throw std::invalid_argument("expected 'e'");

    ++begin;
    return positive ? value : -value;
}

template<bool View>
class str_reader {
public:
    template<typename Iter, typename Size>
    inline string operator ()(Iter &begin, Iter end, Size len) {
        return call(
                begin, end, len,
                typename std::iterator_traits<Iter>::iterator_category()
        );
    }
private:
    template<typename Iter, typename Size>
    string call(Iter &begin, Iter end, Size len, std::forward_iterator_tag) {
        if(std::distance(begin, end) < static_cast<std::ptrdiff_t>(len))
            throw std::invalid_argument("unexpected end of string");

        std::string value(len, 0);
        std::copy_n(begin, len, value.begin());
        std::advance(begin, len);
        return value;
    }

    template<typename Iter, typename Size>
    string call(Iter &begin, Iter end, Size len, std::input_iterator_tag) {
        std::string value(len, 0);
        for(Size i = 0; i < len; i++) {
            if(begin == end)
                throw std::invalid_argument("unexpected end of string");
            value[i] = *begin;
            ++begin;
        }
        return value;
    }
};

template<>
class str_reader<true> {
public:
    template<typename Iter, typename Size>
    string_view operator ()(Iter &begin, Iter end, Size len) {
        if(std::distance(begin, end) < static_cast<std::ptrdiff_t>(len))
            throw std::invalid_argument("unexpected end of string");

        string_view value(&*begin, len);
        std::advance(begin, len);
        return value;
    }
};

template<bool View, typename T>
typename std::conditional<View, string_view, string>::type
decode_str(T &begin, T end) {
    assert(std::isdigit(*begin));
    std::size_t len = 0;
    for(; begin != end && std::isdigit(*begin); ++begin)
        len = len * 10 + *begin - '0';

    if(begin == end)
        throw std::invalid_argument("unexpected end of string");
    if(*begin != ':')
        throw std::invalid_argument("expected ':'");
    ++begin;

    return str_reader<View>{}(begin, end, len);
}

template<bool View, typename T>
typename std::conditional<View, list_view, list>::type
decode_list(T &begin, T end) {
    assert(*begin == 'l');
    ++begin;

    typename std::conditional<View, list_view, list>::type value;
    while(begin != end && *begin != 'e') {
        value.push_back(decode_data<View>(begin, end));
    }

    if(begin == end)
        throw std::invalid_argument("unexpected end of string");

    ++begin;
    return value;
}

template<bool View, typename T>
typename std::conditional<View, dict_view, dict>::type
decode_dict(T &begin, T end) {
    assert(*begin == 'd');
    ++begin;

    typename std::conditional<View, dict_view, dict>::type value;
    while(begin != end && *begin != 'e') {
        if(!std::isdigit(*begin))
            throw std::invalid_argument("expected string token");

        auto k = decode_str<View>(begin, end);
        auto v = decode_data<View>(begin, end);
        auto result = value.emplace(std::move(k), std::move(v));
        if(!result.second) {
            throw std::invalid_argument(
                    "duplicated key in dict: " + std::string(result.first->first)
            );
        }
    }

    if(begin == end)
        throw std::invalid_argument("unexpected end of string");

    ++begin;
    return value;
}

template<bool View, typename T>
typename std::conditional<View, data_view, data>::type
decode_data(T &begin, T end) {
    if(begin == end)
        throw std::invalid_argument("unexpected end of string");

    if(*begin == 'i')
        return decode_int<View>(begin, end);
    else if(*begin == 'l')
        return decode_list<View>(begin, end);
    else if(*begin == 'd')
        return decode_dict<View>(begin, end);
    else if(std::isdigit(*begin))
        return decode_str<View>(begin, end);

    throw std::invalid_argument("unexpected type");
}

}

template<typename T>
inline data decode(T &begin, T end) {
    return detail::decode_data<false>(begin, end);
}

template<typename T>
inline data decode(const T &begin, T end) {
    T b(begin);
    return detail::decode_data<false>(b, end);
}

inline data decode(const string_view &s) {
    return decode(s.begin(), s.end());
}

inline data decode(std::istream &s, eof_behavior e = check_eof) {
    std::istreambuf_iterator<char> begin(s), end;
    auto result = decode(begin, end);
    // If we hit EOF, update the parent stream.
    if(e == check_eof && begin == end)
        s.setstate(std::ios_base::eofbit);
    return result;
}

template<typename T>
inline data_view decode_view(T &begin, T end) {
    return detail::decode_data<true>(begin, end);
}

template<typename T>
inline data_view decode_view(const T &begin, T end) {
    T b(begin);
    return detail::decode_data<true>(b, end);
}

inline data_view decode_view(const string_view &s) {
    return decode_view(s.begin(), s.end());
}

namespace detail {
class list_encoder {
public:
    inline list_encoder(std::ostream &os) : os(os) {
        os.put('l');
    }

    inline ~list_encoder() {
        os.put('e');
    }

    template<typename T>
    inline list_encoder & add(T &&value);
private:
    std::ostream &os;
};

class dict_encoder {
public:
    inline dict_encoder(std::ostream &os) : os(os) {
        os.put('d');
    }

    inline ~dict_encoder() {
        os.put('e');
    }

    template<typename T>
    inline dict_encoder & add(const string_view &key, T &&value);
private:
    std::ostream &os;
};
}

// TODO: make these use unformatted output?
inline void encode(std::ostream &os, integer value) {
    os << "i" << value << "e";
}

inline void encode(std::ostream &os, const string_view &value) {
    os << value.size() << ":" << value;
}

template<typename T>
void encode(std::ostream &os, const std::vector<T> &value) {
    detail::list_encoder e(os);
    for(auto &&i : value)
        e.add(i);
}

template<typename T>
void encode(std::ostream &os, const std::map<string, T> &value) {
    detail::dict_encoder e(os);
    for(auto &&i : value)
        e.add(i.first, i.second);
}

template<typename T>
void encode(std::ostream &os, const std::map<string_view, T> &value) {
    detail::dict_encoder e(os);
    for(auto &&i : value)
        e.add(i.first, i.second);
}

namespace detail {
class encode_visitor : public boost::static_visitor<> {
public:
    inline encode_visitor(std::ostream &os) : os(os) {}

    template<typename T>
    void operator ()(T &&operand) const {
        encode(os, std::forward<T>(operand));
    }
private:
    std::ostream &os;
};
}

// Overload for `data` and `data_view`, but only if the passed-in type is
// already one of the two. Don't accept an implicit conversion!
template<typename T>
auto encode(std::ostream &os, const T &value) ->
typename std::enable_if<
        std::is_same<T, data>::value || std::is_same<T, data_view>::value
>::type {
    boost::apply_visitor(detail::encode_visitor(os), value);
}

namespace detail {
template<typename T>
inline list_encoder & list_encoder::add(T &&value) {
    encode(os, std::forward<T>(value));
    return *this;
}

template<typename T>
inline dict_encoder &
dict_encoder::add(const string_view &key, T &&value) {
    encode(os, key);
    encode(os, std::forward<T>(value));
    return *this;
}
}

template<typename T>
std::string encode(T &&t) {
    std::stringstream ss;
    encode(ss, std::forward<T>(t));
    return ss.str();
}

}

#endif