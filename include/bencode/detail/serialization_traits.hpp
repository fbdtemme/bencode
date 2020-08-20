#pragma once

#include <bencode/detail/bencode_type.hpp>

namespace bencode {


/// Trait class that indicates a type can be serialized to bencode.
/// A valid specialization for type T defines a single static member bvalue of enum type `bencode_type`
/// that specifies the corresponding bencode object type for T.
template<typename T> struct serialization_traits {};


/// Helper classes to create specializations of `serialisation_traits`.
/// @example:
///     template<> struct serialisation_traits<MyType> : serializes_to_integer {};
struct serializes_to_runtime_type { static constexpr auto type = bencode_type::uninitialized; };
struct serializes_to_integer      { static constexpr auto type = bencode_type::integer; };
struct serializes_to_string       { static constexpr auto type = bencode_type::string; };
struct serializes_to_list         { static constexpr auto type = bencode_type::list;};
struct serializes_to_dict         { static constexpr auto type = bencode_type::dict; };

/// A type T satisfied the serialisable concept if it has a valid specialization of `serialisation_traits`.
template <typename T>
concept serializable =
    requires() {
        { serialization_traits<T>::type } -> std::convertible_to<bencode_type>;
    };

/// A type T satisfied the serializable_to concept if it is serializable
/// and has a serialization_trait<T>::type == BencodeType.
template <typename T, bencode_type BencodeType>
concept serializable_to =
    serializable<T> &&
    (serialization_traits<T>::type == BencodeType);


} // namespace bencode

#define BENCODE_SERIALIZES_TO(BENCODE_TYPE, TYPE)                                     \
template <> struct bencode::serialization_traits<TYPE> : serializes_to_##BENCODE_TYPE {}; \

#define BENCODE_SERIALIZES_TO_RUNTIME_TYPE(...) \
BENCODE_SERIALIZES_TO(runtime_type, __VA_ARGS__)

#define BENCODE_SERIALIZES_TO_INTEGER(...) \
BENCODE_SERIALIZES_TO(integer, __VA_ARGS__)

#define BENCODE_SERIALIZES_TO_STRING(...) \
BENCODE_SERIALIZES_TO(string, __VA_ARGS__)

#define BENCODE_SERIALIZES_TO_LIST(...) \
BENCODE_SERIALIZES_TO(list, __VA_ARGS__)

#define BENCODE_SERIALIZES_TO_DICT(...) \
BENCODE_SERIALIZES_TO(dict, __VA_ARGS__)

