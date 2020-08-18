#pragma once

#include <unordered_map>
#include "bencode/detail/serialization_traits.hpp"

namespace bencode {

template <
        class Key,
        class T,
        class Hash,
        class KeyEqual,
        class Allocator
>
struct serialization_traits<std::unordered_map<Key, T, Hash, KeyEqual, Allocator> >      : serializes_to_dict {};
template <
        class Key,
        class T,
        class Hash,
        class KeyEqual,
        class Allocator
>
struct serialization_traits<std::unordered_multimap<Key, T, Hash, KeyEqual, Allocator> > : serializes_to_dict {};

}