#pragma once

#include <map>
#include "bencode/detail/serialization_traits.hpp"

namespace bencode {

template <typename Key, typename T, typename Compare, typename Alloc>
struct serialization_traits<std::map<Key, T, Compare, Alloc> >     : serializes_to_dict {};


template <typename Key, typename T, typename Compare, typename Alloc>
struct serialization_traits<std::multimap<Key, T, Compare, Alloc> > : serializes_to_dict {};

}
