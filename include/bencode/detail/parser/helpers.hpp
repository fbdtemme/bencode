#pragma once
#include <cstdint>
#include <type_traits>
#include <concepts>

#if defined (_MSC_VER)
#include <bencode/detail/safe_int.hpp>
#endif
namespace bencode::detail
{

#if defined (__GNUC__) || defined(__CLANG__)
template <std::integral T>
constexpr bool raise_and_add_safe(T& value, std::type_identity_t<T> base, std::type_identity_t<T> c)
{
    if (__builtin_mul_overflow(value, base, &value) ||
        __builtin_add_overflow(value, c, &value))
        return false;
    return true;
}
#endif

#if defined (_MSC_VER)
template <std::integral T>
constexpr bool raise_and_add_safe(T& value, std::type_identity_t<T> base, std::type_identity_t<T> c)
{
    if (SafeMultiply(value, base, value) && SafeAdd(value, c, value)) {
        return true;
    }
    return false;
}
#endif


template <std::integral T>
constexpr void raise_and_add(T& value1, std::type_identity_t<T> base, std::type_identity_t<T> value2)
{
    value1 = value1 * base + value2;
}
}