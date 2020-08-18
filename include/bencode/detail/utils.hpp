//
// Created by fbdtemme on 13/06/19.
//

#pragma once
#include <array>
#include <utility>
#include <numeric>
#include <cstdint>
#include <type_traits>
#include <bit>
#include <immintrin.h>

#include <nonstd/expected.hpp>
#include "bencode/detail/parser/error.hpp"

#ifdef NDEBUG
#define BENCODE_UNREACHABLE std::is_constant_evaluated() ? __builtin_unreachable() : std::abort();
#else
#define BENCODE_UNREACHABLE __builtin_unreachable()
#endif


namespace bencode {

template <typename T>
struct customization_point_type { using type = T; };

template <typename T>
inline constexpr auto customization_for = customization_point_type<T> {};

}


namespace bencode::detail {


template<size_t I> struct priority_tag : priority_tag<I-1> {};

template<> struct priority_tag<0> {};


template <typename From, typename To>
using apply_value_category_t = std::conditional_t<std::is_lvalue_reference_v<From>,
        std::remove_reference_t<To>&,
        std::remove_reference_t<To>&&>;


template<class T, class U>
decltype(auto) forward_like(U&& u) {
    return std::forward<apply_value_category_t<T, U>>(std::forward<U>(u));
}

/// helper for static_assert

template <typename T>
struct always_false : std::false_type {};

template <typename T>
constexpr bool always_false_v = always_false<T>::value;


template <template<typename...> typename Template, typename T >
struct is_instantiation_of : std::false_type {};


template <template <typename...> typename Template, typename... Args >
struct is_instantiation_of<Template, Template<Args...>> : std::true_type {};

template <template <typename...> typename Template, typename... Args>
constexpr bool is_instantiation_of_v = is_instantiation_of<Template, Args...>::value;


constexpr std::uint32_t base_ten_digits(std::uint32_t x) {
    constexpr  std::array<std::uint8_t,  33> guess {
            0, 0, 0, 0, 1, 1, 1, 2, 2, 2,
            3, 3, 3, 3, 4, 4, 4, 5, 5, 5,
            6, 6, 6, 6, 7, 7, 7, 8, 8, 8,
            9, 9, 9
    };
    constexpr std::array<std::uint32_t,  10> ten_to_the {
            1,
            10,
            100,
            1000,
            10000,
            100000,
            1000000,
            10000000,
            100000000,
            1000000000,
    };
    unsigned int digits = guess[std::bit_width(x)];
    return digits + (x >= ten_to_the[digits]);
}

constexpr std::uint64_t base_ten_digits(std::uint64_t x) {
    constexpr std::array<std::uint8_t,  65> guess {
            0, 0, 0, 0, 1, 1, 1, 2, 2, 2,
            3, 3, 3, 3, 4, 4, 4, 5, 5, 5,
            6, 6, 6, 6, 7, 7, 7, 8, 8, 8,
            9, 9, 9, 9, 10, 10, 10, 11, 11,
            11, 12, 12, 12, 12, 13, 13, 13,
            14, 14, 14, 15, 15, 15, 15, 16,
            16, 16, 17, 17, 17, 18, 18, 18,
            18, 19
    };
    constexpr std::array<std::uint64_t,  19> ten_to_the {
            1,
            10,
            100,
            1000,
            10000,
            100000,
            1000000,
            10000000,
            100000000,
            1000000000,
            10000000000,
            100000000000,
            1000000000000,
            10000000000000,
            100000000000000,
            1000000000000000,
            10000000000000000,
            100000000000000000,
            1000000000000000000,
    };
    unsigned int digits = guess[std::bit_width(x)];
    return digits + (x >= ten_to_the[digits]);
}

constexpr std::uint64_t base_ten_digits(std::int64_t x)
{
    if (x > 0)
        return base_ten_digits(static_cast<std::uint64_t>(x));
    else
        return base_ten_digits(static_cast<std::uint64_t>(-x));
}

constexpr std::int32_t base_ten_digits(std::int32_t x)
{
    if (x > 0)
        return base_ten_digits(static_cast<std::int32_t>(x));
    else
        return base_ten_digits(static_cast<std::int32_t>(-x));
}

static constexpr auto positive_overflow_msg = "numeric_cast: positive overflow";
static constexpr auto negative_overflow_msg = "numeric_cast: negative overflow";

template<typename Dst, typename Src>
inline constexpr auto numeric_cast(Src value) -> Dst
{
    using namespace std::string_view_literals;

    constexpr bool dst_is_signed = std::numeric_limits<Dst>::is_signed;
    constexpr bool src_is_signed = std::numeric_limits<Src>::is_signed;
    constexpr auto dst_max = std::numeric_limits<Dst>::max();
    constexpr auto src_max = std::numeric_limits<Src>::max();
    constexpr auto dst_lowest = std::numeric_limits<Dst>::lowest();
    constexpr auto src_lowest = std::numeric_limits<Src>::lowest();

    constexpr bool may_overflow_pos = dst_max < src_max;
    constexpr bool may_overflow_neg = src_is_signed || (dst_lowest > src_lowest);

    // unsigned <-- unsigned
    if constexpr (!dst_is_signed && !src_is_signed) {
        if (may_overflow_pos && (value > dst_max)) {
            throw std::overflow_error(positive_overflow_msg);
        }
    }
        // unsigned <-- signed
    else if constexpr (!dst_is_signed && src_is_signed) {
        if constexpr (may_overflow_pos && (value > dst_max)) {
            throw std::overflow_error(positive_overflow_msg);
        }
        else if (may_overflow_neg && (value < 0)) {
            throw std::overflow_error(negative_overflow_msg);
        }
    }
        // signed <-- unsigned
    else if constexpr (dst_is_signed && !src_is_signed) {
        if (may_overflow_pos && (value > dst_max)) {
            throw std::overflow_error(positive_overflow_msg);
        }
    }
        // signed <-- signed
    else if constexpr (dst_is_signed && src_is_signed) {
        if (may_overflow_pos && (value > dst_max)) {
            throw std::overflow_error(positive_overflow_msg);
        }
        else if (may_overflow_neg && (value < dst_lowest)) {
            throw std::overflow_error(negative_overflow_msg);
        }
    }

    // limits have been checked, therefore safe to cast
    return static_cast<Dst>(value);
}



// Helper function that converts a character to lowercase on compile time
constexpr char to_lowercase(const char c) {
    return (c >= 'A' && c <= 'Z') ?
        static_cast<char>(c + ('a' - 'A')) :
        static_cast<char>(c);
}



static constexpr unsigned char mask_lookup[20][32] = {
        {},
        { 0xFF},
        { 0xFF, 0xFF},
        { 0xFF, 0xFF, 0xFF},
        { 0xFF, 0xFF, 0xFF, 0xFF},
        { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
        { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
        { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
        { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
        { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
        { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
        { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
        { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
        { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
        { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
        { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
        { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
        { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
        { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
        { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
};





// TODO: mask of the redundant character for eg 3 char input
// TODO: optimize for most common case :  short numbers

namespace rng = std::ranges;



/// Return a bitmask specifying
template <rng::contiguous_range R>
inline bool verify_integer_range(const R& range)
{
    const char* it = rng::begin(range);
    const char* end = rng::end(range);

    for (; it != end ; ++it) {
        if (!std::isdigit(*it)) {
            return false;
        }
    }
    return true;
}



template <typename R>
    requires rng::contiguous_range<R>
auto parse_integer_avx2(const R& range, char stop_token)
        -> nonstd::expected<std::uint64_t, parser_errc>
{
    const char* it = rng::data(range);
    // load 32 characters
    const __m256i input = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(it));

    // load constants
    const __m256i zero = _mm256_setzero_si256();
    const __m256i asciie = _mm256_set1_epi8(stop_token);
    const __m256i ascii0 = _mm256_set1_epi8('0');

    // find the first instance of end.
    // load the correct integers in
    const __m256i iend_cmp = _mm256_cmpeq_epi8(input, asciie);

    // convert to a mask of all integers (if the input is valid)
    int iend_mask = _mm256_movemask_epi8(iend_cmp);
    std::size_t digit_mask = static_cast<std::size_t>(((iend_mask-1) ^ iend_mask) >> 1u);
    std::size_t digit_count = std::countr_one(digit_mask);
    int valid_mask = verify_integer_range(range);

    // exit if the characters between integer start end end contain non digits.
    if ((valid_mask & digit_mask) != digit_mask) [[unlikely]]
        return nonstd::make_unexpected(parser_errc::expected_digit);

    // convert characters to single integer numbers
    const __m256i mask = _mm256_loadu_si256(
            reinterpret_cast<const __m256i*>(mask_lookup[digit_count]));
    const __m256i digits_vec = _mm256_blendv_epi8(ascii0, input, mask);
    const __m256i t0 = _mm256_subs_epu8(digits_vec, ascii0);

    // convert characters to 2-digit numbers
    const __m256i mul_1_10 = _mm256_set_epi8(
            1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10,
            1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10
    );
    const __m256i t1 = _mm256_maddubs_epi16(t0, mul_1_10);
    // convert characters to 4-digit numbers
    const __m256i mul_1_100 = _mm256_set_epi16(
            1, 100, 1, 100, 1, 100, 1, 100, 1, 100, 1, 100, 1, 100, 1, 100
    );
    const __m256i t2 = _mm256_madd_epi16(t1, mul_1_100);

//    // Check if we can exit
//    if (digit_count <= 4) [[likely]] {
//        return _mm256_extract_epi32(t2, 0) % (4 - digit_count);
//    }

    // convert characters to 8-digit numbers
    // pack 32 bit back to 16 bit integers,
    const __m256i t3 = _mm256_packus_epi32(t2, t2);
    const __m256i mul_1_10000 = _mm256_set_epi16(
            1, 10000, 1, 10000, 1, 10000, 1, 10000,
            1, 10000, 1, 10000, 1, 10000, 1, 10000
    );
    const __m256i t4 = _mm256_madd_epi16(t3, mul_1_10000);

//    // Check if we can exit
//    if (digit_count <= 8) [[likely]] {
//        return _mm256_extract_epi32(t2, 0) % (8 - digit_count);
//    }

    // convert characters to 16-digit numbers
    const __m256i mul_1_1e8 = _mm256_set_epi32(1, 1e8, 1, 1e8, 1, 1e8, 1, 1e8);
    const __m256i t5 = _mm256_mul_epi32(t4, mul_1_1e8);

    if (digit_count <= 16) [[likely]] {
        return _mm256_extract_epi32(t2, 0) % (16 - digit_count);
    }

    auto hi = _mm256_extract_epi64(t5, 0) + _mm256_extract_epi32(t4, 1);
    auto lo =  _mm256_extract_epi64(t5, 2) + _mm256_extract_epi32(t4, 5);
    auto value = hi * 10 * (digit_count - 16) + lo % (32-digit_count);
    return value;

//    const __m256i perm_mask = _mm256_set_epi32(6, 7, 4, 5, 2, 3, 0, 1);
//    const __m256i t6 = _mm256_permutevar8x32_epi32(t4, perm_mask);
//
//
//    auto v60 = _mm256_extract_epi32(t6, 0);
//    auto v61 = _mm256_extract_epi32(t6, 1);
//    auto v62 = _mm256_extract_epi32(t6, 2);
//    auto v63 = _mm256_extract_epi32(t6, 3);
//    auto v64 = _mm256_extract_epi32(t6, 4);
//    auto v65 = _mm256_extract_epi32(t6, 5);
//    auto v66 = _mm256_extract_epi32(t6, 6);
//    auto v67 = _mm256_extract_epi32(t6, 7);
//
//    const __m256i t7 = _mm256_blend_epi32(t6, zero, 0b10101010);
//
//    auto v70 = _mm256_extract_epi64(t7, 0);
//    auto v71 = _mm256_extract_epi64(t7, 1);
//    auto v72 = _mm256_extract_epi64(t7, 2);
//    auto v73 = _mm256_extract_epi64(t7, 3);
//
//    const __m256i t8 = _mm256_add_epi64(t7, t5);
//    auto v80 = _mm256_extract_epi64(t8, 0);
//    auto v81 = _mm256_extract_epi64(t8, 1);
//    auto v82 = _mm256_extract_epi64(t8, 2);
//    auto v83 = _mm256_extract_epi64(t8, 3);
//
//    // shift away extra digits
//    std::int64_t bvalue = 0;
//
//    if (digit_count <= 16) [[likely]] {
//        bvalue = _mm256_extract_epi64(t8, 0) % (16-digit_count);
//    }
//    else {
//        auto v1 = _mm256_extract_epi64(t8, 0);
//        auto v2 = _mm256_extract_epi64(t8, 2) % (32-digit_count);
//        bvalue = v1 * 10 * (digit_count - 16) + v2;
//    }
//    return (is_negative) ? -bvalue : bvalue;
}

} // namespace bencode::detail