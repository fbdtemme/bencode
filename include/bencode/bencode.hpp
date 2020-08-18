#pragma once


#include "bencode/detail/bvalue/basic_bvalue.hpp"
#include "bencode/detail/bvalue/concepts.hpp"
#include "bencode/detail/bvalue/accessors.hpp"
#include "bencode/detail/bvalue/assignment.hpp"
#include "bencode/detail/bvalue/conversion.hpp"
#include "bencode/detail/bvalue/comparison.hpp"
#include "bencode/detail/bvalue/events.hpp"
#include "bencode/detail/decode_value.hpp"

#include "bencode/detail/bview/bview.hpp"
#include "bencode/detail/bview/integer_bview.hpp"
#include "bencode/detail/bview/string_bview.hpp"
#include "bencode/detail/bview/list_bview.hpp"
#include "bencode/detail/bview/dict_bview.hpp"
#include "bencode/detail/bview/accessors.hpp"
#include "bencode/detail/bview/conversion.hpp"
#include "bencode/detail/bview/comparison.hpp"
#include "bencode/detail/bview/events.hpp"
#include "bencode/detail/bview/to_bvalue.hpp"
#include "bencode/detail/decode_view.hpp"


// traits
#include "bencode/traits/fundamentals.hpp"

// encoding decoding

#include "bencode/detail/encode.hpp"
#include "bencode/detail/decode_value.hpp"
#include "bencode/detail/encoding_ostream.hpp"

