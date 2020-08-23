

///@file forward declarations

namespace bencode {

class bview;
class integer_bview;
class string_bview;
class list_bview;
class dict_bview;

class descriptor;
class descriptor_table;

class bad_access;
class bad_bview_access;
class bad_bview_access;
class parsing_error;
class conversion_error;


template<
        typename IntegralType,
        typename StringViewType,
        typename StringType,
        template <typename T> typename ListType,
        template <typename K, typename V> typename DictType
>
struct bvalue_policy;

struct default_bvalue_policy;
template <typename Policy> class basic_bvalue;


template <typename Iterator = const char*, typename Sentinel = Iterator>
class descriptor_parser;

template <typename Iterator = const char*, typename Sentinel = Iterator>
class push_parser;

}