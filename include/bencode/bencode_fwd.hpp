

///@file forward declarations

namespace bencode {

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

class bview;
class integer_bview;
class string_bview;
class list_bview;
class dict_bview;

class bpointer;

class bad_access;
class bad_bview_access;
class bad_bvalue_access;
class parsing_error;
class conversion_error;
class bpointer_error;

class descriptor;
class descriptor_table;

template <typename Iterator, typename Sentinel>
class descriptor_parser;

template <typename Iterator, typename Sentinel>
class push_parser;

}