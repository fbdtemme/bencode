#pragma once
#include <cstdint>

namespace bencode {

template<
        typename IntegralType,
        typename StringViewType,
        typename StringType,
        template <typename T> typename ListType,
        template <typename K, typename V> typename DictType
>
struct bvalue_policy;

template <typename Policy>
class basic_bvalue;

enum class bencode_type;

template <typename T>
struct traits;
template <typename T>
struct serialization_traits;

struct default_bvalue_policy;

class descriptor;
class integer_bview;
class string_bview;
class list_bview;
class dict_bview;


// Error handling

class parse_error;
class bad_access;
class bad_bvalue_access;
class bad_bview_access;

// "bencode/detail/exceptions.hpp"
enum class conversion_errc : std::uint8_t;



namespace events {
namespace consumer { }
namespace producer { }
}

} // namespace bencode


