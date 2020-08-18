//#pragma once
//#include <ranges>
//#include <concepts>
//#include <immintrin.h>
//#include <nonstd/expected.hpp>
//#include <bit>
//
//#include "bencode/detail/parser/error.hpp"
//
//static constexpr unsigned char digit_mask_lut[20][32] = {
//        {},
//        { 0xFF},
//        { 0xFF, 0xFF},
//        { 0xFF, 0xFF, 0xFF},
//        { 0xFF, 0xFF, 0xFF, 0xFF},
//        { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
//        { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
//        { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
//        { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
//        { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
//        { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
//        { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
//        { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
//        { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
//        { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
//        { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
//        { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
//        { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
//        { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
//        { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
//};
//
//
//
//
//namespace bencode::sse {
//
//std::uint32_t parse_eight_digits_unrolled(const char* chars)
//{
//    __m128i ascii0 = _mm_set1_epi8('0');
//    __m128i mul_1_10 = _mm_setr_epi8(10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1);
//    __m128i mul_1_100 = _mm_setr_epi16(100, 1, 100, 1, 100, 1, 100, 1);
//    __m128i mul_1_10000 = _mm_setr_epi16(10000, 1, 10000, 1, 10000, 1, 10000, 1);
//    __m128i in = _mm_sub_epi8(_mm_loadu_si128((__m128i*) chars), ascii0);
//    __m128i t1 = _mm_maddubs_epi16(in, mul_1_10);
//    __m128i t2 = _mm_madd_epi16(t1, mul_1_100);
//    __m128i t3 = _mm_packus_epi32(t2, t2);
//    __m128i t4 = _mm_madd_epi16(t3, mul_1_10000);
//    return _mm_cvtsi128_si32(t4);
//}
//}
//
//
//namespace bencode::avx2 {
//
//namespace rng = std::ranges;
//
//
//__m256i decimal_digits_mask_v1(const __m256i input) {
//     // check if all values in this range are integers.
//    __m256i c0 = _mm256_cmpgt_epi8(input, _mm256_set1_epi8('0'-1));     // t1 = (x >= '0')
//    __m256i c1 = _mm256_cmpgt_epi8(input,  _mm256_set1_epi8('9'));      // t0 = (x > '9')
//    __m256i digits_mask = _mm256_andnot_si256(c1, c0);
//    return digits_mask;
//}
//
//__m256i decimal_digits_mask_v2(const __m256i input) {
//    // Convert ascii code points '0'-'9' [48-57] to the range [117-127] to check for digits
//    // with a single comparison
//    const __m256i t0 = _mm256_add_epi8(input, _mm256_set1_epi8(79));
//    __m256i digits_mask = _mm256_cmpgt_epi8(t0, _mm256_set1_epi8(116));
//    return digits_mask;
//}
//
//
//
//inline std::size_t decimal_digits_bitmask(const __m256i input)
//{
//    // Convert ascii code points '0'-'9' [48-57] to the range [117-127] to check for digits
//    // with a single comparison
//    __m256i t0 = _mm256_add_epi8(input, _mm256_set1_epi8(70));
//    __m256i digits_mask = _mm256_cmpgt_epi8(t0, _mm256_set1_epi8(117));
//    return _mm256_movemask_epi8(digits_mask);
//}
//
//
//
//inline uint64_t prefix_xor(const uint64_t bitmask) {
//    __m128i all_ones = _mm_set1_epi8('\xFF');
//    __m128i result = _mm_clmulepi64_si128(_mm_set_epi64x(0ULL, bitmask), all_ones, 0);
//    return _mm_cvtsi128_si64(result);
//}
//
//
//// Classify tokens.
////  | codepoint | acii   |
////  |--------------------|
////  | 45        | -      |
////  | 48..57    | [0-9]  |
////  | 58        | :      |
////  | 100       | d      |
////  | 101       | e      |
////  | 108       | l      |
////  ----------------------
//
//
//inline std::uint32_t integer_token_mask(const __m256i input)
//{
//    __m256i reverse_input = reverse_input(input);
//    __m256i t0 = _mm256_add_epi8(input, _mm256_set1_epi8(70));
//    __m256i digits_mask = _mm256_cmpgt_epi8(t0, _mm256_set1_epi8(117));
//    __m256i minus_mask  = _mm256_cmpeq_epi8(input, _mm256_set1_epi8('-'));
//    __m256i digits_or_minus = _mm256_or_si256(digits_mask, minus_mask);
//    __m256i reverse_digits_or_minus = reverse_char_vector(digits_or_minus);
//
//    __m256i i_mask = _mm256_cmpeq_epi8(input, _mm256_set1_epi8('i'));
//    __m256i reverse_e_mask = _mm256_cmpeq_epi8(reverse_input, _mm256_set1_epi8('e'));
//
//    std::uint32_t i_bitmask = _mm256_movemask_epi8(i_mask);
//    std::uint32_t r_e_bitmask = _mm256_movemask_epi8(reverse_e_mask);
//    std::uint32_t d_or_m_bitmask = _mm256_movemask_epi8(digits_or_minus);
//    std::uint32_t r_d_or_m_bitmask = _mm256_movemask_epi8(reverse_digits_or_minus);
//
//    auto i_carry_through = ((d_or_m_bitmask + (i_bitmask << 1)) ^ d_or_m_bitmask) | i_bitmask;
//    auto r_e_carry_through = ((r_d_or_m_bitmask) + (r_e_bitmask << 1) ^ r_d_or_m_bitmask) | r_e_bitmask;
//    auto e_reverse_carry = reverse_bits(r_e_carry_through);
//    auto int_bitmask = i_carry_through & e_reverse_carry;
//
//    return int_bitmask;
//}
//
//
//inline std::uint32_t string_length_token_mask(const __m256i input)
//{
//    __m256i reverse_input = reverse_char_vector(input);
//    __m256i rev_semicolon_mask = _mm256_cmpeq_epi8(reverse_input, _mm256_set1_epi8(':'));
//    __m256i digits_mask = _mm256_cmpgt_epi8(input, _mm256_set1_epi8(117));
//    __m256i minus_mask  = _mm256_cmpeq_epi8(input, _mm256_set1_epi8('-'));
//    __m256i digits_or_minus = _mm256_or_si256(digits_mask, minus_mask);
//    __m256i rev_digits_or_minus = reverse_char_vector(digits_or_minus);
//
//    std::uint32_t rev_semicolon_bmask = _mm256_movemask_epi8(rev_semicolon_mask);
//    std::uint32_t rev_numerical_bmask = _mm256_movemask_epi8(rev_digits_or_minus);
//
//    std::uint32_t rev_string_len_bmask = ((rev_numerical_bmask) + (rev_semicolon_bmask << 1) ^ rev_numerical_bmask) | rev_semicolon_bmask;
//    auto string_len_bmask = reverse_bits(rev_string_len_bmask);
//    return string_len_bmask;
//}
//
//
//
//
////
////inline std::size_t find_integer_ranges(std::string_view s)
////{
////    // load 32 characters
////    __m256i ascii_start = _mm256_set1_epi8('i');
////    __m256i ascii_stop =_mm256_set1_epi8('e');
////
////    // check if all values in this range are integers.
////    __m256i i_bits = _mm256_cmpeq_epi8(in, ascii_start);
////    __m256i e_bits = _mm256_cmpeq_epi8(in, ascii_stop);
////
////    // t1 = (x >= '0')
////    __m256i c1 = _mm256_cmpgt_epi8(in, ascii9);            // t0 = (x > '9')
////    __m256i valid_vec = _mm256_andnot_si256(c1, c0);
////    return _mm256_movemask_epi8(valid_vec);
////}
//
//inline std::size_t get_valid_integer_mask(const __m256i& in)
//{
//    // load 32 characters
//    __m256i before_ascii0 = _mm256_set1_epi8('0'-1);
//    __m256i ascii9 = _mm256_set1_epi8('9');
//
//    // check if all values in this range are integers.
//    __m256i c0 = _mm256_cmpgt_epi8(in, before_ascii0);     // t1 = (x >= '0')
//    __m256i c1 = _mm256_cmpgt_epi8(in, ascii9);            // t0 = (x > '9')
//    __m256i valid_vec = _mm256_andnot_si256(c1, c0);
//    return _mm256_movemask_epi8(valid_vec);
//}
//
//
///// Return a bitmask specifying
//inline std::size_t find_range_end(const __m256i& in, char stop_token)
//{
//    // load constants
//    __m256i zero = _mm256_setzero_si256();
//    __m256i ascii_stop = _mm256_set1_epi8(stop_token);
//
//    // find the first instance of end.
//    // load the correct integers in
//    __m256i end_cmp = _mm256_cmpeq_epi8(in, ascii_stop);
//
//    // convert to a mask of all integers (if the input is valid)
//    int iend_mask = _mm256_movemask_epi8(end_cmp);
//    std::size_t digit_mask = ((iend_mask-1) ^ iend_mask) >> 1u;
//    return digit_mask;
//}
//
///// Parse a string representation of an integer.
//template <rng::input_range R>
//inline auto parse_integer_mixed(R& range) -> nonstd::expected<std::int64_t, parser_errc>
//{
//    constexpr auto max_digits = 19;
//    bool negative = false;
//    std::int64_t bvalue {};
//    auto it = rng::data(range);
//    auto end = rng::data(range) + rng::distance(range);
//
//    if (*it == '-') {
//        negative = true;
//        ++it;
//    }
//
//    // do not advance here since we must read the zero again
//    const bool leading_zero = (*it == '0');
//    const __m256i input = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(it));
//    // validate input with simd, this simplifies the loop condition to a single variable.
//    std::size_t range_mask = find_range_end(input, 'e');
//    int valid_mask = get_valid_integer_mask(input);
//
//    // exit if the characters between integer start end end contain non digits.
//    if ((valid_mask & range_mask) != range_mask) [[unlikely]]
//        return nonstd::make_unexpected(parser_errc::expected_digit);
//
//    std::size_t digit_count = std::countr_one(range_mask);
//    if (digit_count > max_digits) [[unlikely]] {
//        return nonstd::make_unexpected(parser_errc::integer_overflow);
//    }
//
//    for (std::size_t i = 0; i < digit_count; ++i, ++it) {
//        bvalue = 10 * bvalue + static_cast<unsigned>(*it)-'0';
//    }
//
//    if (digit_count == max_digits && bvalue < 1e19) [[unlikely]] {
//        return nonstd::make_unexpected(parser_errc::integer_overflow);
//    }
//    if (leading_zero && bvalue != 0) [[unlikely]] {
//        return nonstd::make_unexpected(parser_errc::leading_zero);
//    }
//    if (negative && bvalue == 0) [[unlikely]] {
//        return nonstd::make_unexpected(parser_errc::negative_zero);
//    }
//    return negative ? -bvalue : bvalue;
//}
//
//
//
//template <rng::contiguous_range R>
//inline nonstd::expected<std::uint64_t, parser_errc>
//parse_integer_full(const R& range)
//{
//    const char* it = rng::data(range);
//    const bool leading_zero = (*it == '0');
//    const bool negative = (*it == '-');
//    std::int64_t bvalue = 0;
//
//    const __m256i input = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(it));
//    std::size_t range_mask = find_range_end(input, 'e');
//    std::size_t valid_mask = get_valid_integer_mask(input);
//
//    // exit if the characters between integer start end end contain non digits.
//    if ((valid_mask & range_mask) != range_mask) [[unlikely]]
//        return nonstd::make_unexpected(parser_errc::expected_digit);
//
//    std::size_t digit_count = std::countr_one(range_mask);
//    __m256i digit_mask = _mm256_loadu_si256(
//            reinterpret_cast<const __m256i*>(digit_mask_lut[digit_count]));
//    // convert characters to single integer numbers
//    __m256i ascii0 = _mm256_set1_epi8('0');
//    __m256i digits_vec = _mm256_blendv_epi8(ascii0, input, digit_mask);
//    __m256i t0 = _mm256_subs_epu8(digits_vec, ascii0);
//
//    // convert characters to 2-digit numbers
//    __m256i mul_1_10 = _mm256_set_epi8(
//            1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10,
//            1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10
//    );
//    __m256i t1 = _mm256_maddubs_epi16(t0, mul_1_10);
//    // convert characters to 4-digit numbers
//    __m256i mul_1_100 = _mm256_set_epi16(
//            1, 100, 1, 100, 1, 100, 1, 100, 1, 100, 1, 100, 1, 100, 1, 100
//    );
//    __m256i t2 = _mm256_madd_epi16(t1, mul_1_100);
//
//    // Check if we can exit
//    if (digit_count < 4) [[likely]] {
//        bvalue = _mm256_extract_epi32(t2, 0) % (4 - digit_count);
//        return (negative) ? -bvalue : bvalue;
//    }
//    if (digit_count == 4) [[likely]] {
//        bvalue =  _mm256_extract_epi32(t2, 0);
//        return (negative) ? -bvalue : bvalue;
//    }
//        // convert characters to 8-digit numbers
//    // pack 32 bit back to 16 bit integers,
//    __m256i t3 = _mm256_packus_epi32(t2, t2);
//    __m256i mul_1_10000 = _mm256_set_epi16(
//            1, 10000, 1, 10000, 1, 10000, 1, 10000,
//            1, 10000, 1, 10000, 1, 10000, 1, 10000
//    );
//    __m256i t4 = _mm256_madd_epi16(t3, mul_1_10000);
//
//    // Check if we can exit
//    if (digit_count < 8) [[likely]] {
//        bvalue = _mm256_extract_epi32(t2, 0) % (8 - digit_count);
//        return (negative) ? -bvalue : bvalue;
//    }
//    if (digit_count == 8) [[likely]] {
//        bvalue = _mm256_extract_epi32(t2, 0);
//        return (negative) ? -bvalue : bvalue;
//    }
//
//    // convert characters to 16-digit numbers
//    __m256i mul_1_1e8 = _mm256_set_epi32(1, 1e8, 1, 1e8, 1, 1e8, 1, 1e8);
//    __m256i t5 = _mm256_mul_epi32(t4, mul_1_1e8);
//
//    if (digit_count < 16) [[likely]] {
//        bvalue = _mm256_extract_epi32(t2, 0) % (16 - digit_count);
//        return (negative) ? -bvalue : bvalue;
//    }
//
////    auto hi = _mm256_extract_epi64(t5, 0) + _mm256_extract_epi32(t4, 1);
////    auto lo =  _mm256_extract_epi64(t5, 2) + _mm256_extract_epi32(t4, 5);
////    auto bvalue = hi * 10 * (digit_count - 16) + lo % (32-digit_count);
////    return bvalue;
////
////    auto v5_0 = _mm256_extract_epi64(t5, 0);
////    auto v5_1 = _mm256_extract_epi64(t5, 1);
////    auto v5_2 = _mm256_extract_epi64(t5, 2);
////    auto v5_3 = _mm256_extract_epi64(t5, 3);
//////
////    auto t4_0 = _mm256_extract_epi32(t4, 0);
////    auto t4_1 = _mm256_extract_epi32(t4, 1);
////    auto t4_2 = _mm256_extract_epi32(t4, 2);
////    auto t4_3 = _mm256_extract_epi32(t4, 3);
////    auto t4_4 = _mm256_extract_epi32(t4, 4);
////    auto t4_5 = _mm256_extract_epi32(t4, 5);
////    auto t4_6 = _mm256_extract_epi32(t4, 6);
////    auto t4_7 = _mm256_extract_epi32(t4, 7);
//
//    const __m256i perm_mask = _mm256_set_epi32(6, 7, 4, 5, 2, 3, 0, 1);
//    __m256i t6 = _mm256_permutevar8x32_epi32(t4, perm_mask);
//
//    const __m256i mul_1_0 = _mm256_set_epi32(0, 1, 0, 1, 0, 1, 0, 1);
//    __m256i t7 = _mm256_mullo_epi32(t6, mul_1_0);
//
////    auto v70 = _mm256_extract_epi32(t7, 0);
////    auto v71 = _mm256_extract_epi32(t7, 1);
////    auto v72 = _mm256_extract_epi32(t7, 2);
////    auto v73 = _mm256_extract_epi32(t7, 3);
////    auto v74 = _mm256_extract_epi32(t7, 4);
////    auto v75 = _mm256_extract_epi32(t7, 5);
////    auto v76 = _mm256_extract_epi32(t7, 6);
////    auto v77 = _mm256_extract_epi32(t7, 7);
////
////    auto v7_64_0 = _mm256_extract_epi64(t7, 0);
////    auto v7_64_1 = _mm256_extract_epi64(t7, 1);
////    auto v7_64_2 = _mm256_extract_epi64(t7, 2);
////    auto v7_64_3 = _mm256_extract_epi64(t7, 3);
//
////    __m256i tunpack = _mm256_unpacklo_epi32(t4, t4);
////    auto vtu0 = _mm256_extract_epi64(tunpack, 0);
////    auto vtu1 = _mm256_extract_epi64(tunpack, 1);
////    auto vtu2 = _mm256_extract_epi64(tunpack, 2);
////    auto vtu3 = _mm256_extract_epi64(tunpack, 3);
////    __m256i zero = _mm256_set1_epi8(0);
////    __m256i t7 = _mm256_blend_epi32(t6, zero, 0b10101010);
////
//
////
//    const __m256i t8 = _mm256_add_epi64(t7, t5);
////    auto v80 = _mm256_extract_epi64(t8, 0);
////    auto v81 = _mm256_extract_epi64(t8, 1);
////    auto v82 = _mm256_extract_epi64(t8, 2);
////    auto v83 = _mm256_extract_epi64(t8, 3);
//
//    std::int64_t v0 = _mm256_extract_epi64(t8, 0);
//    std::int64_t v1 = _mm256_extract_epi64(t8, 2);
//    if (digit_count == 16) [[likely]] {
//        bvalue = _mm256_extract_epi32(t2, 0);
//        return (negative) ? -bvalue : bvalue;
//    } else {
//        bvalue = v0 * 10 * (digit_count-16) + v1 % (32-digit_count);
//    }
//    return (negative) ? -bvalue : bvalue;
//}
//
//template <rng::contiguous_range R>
//inline nonstd::expected<std::uint64_t, parser_errc>
//parse_integer_full_v2(const R& range)
//{
//    const char* it = rng::data(range);
//    const bool leading_zero = (*it == '0');
//    const bool negative = (*it == '-');
//    std::int64_t bvalue = 0;
//
//    const __m256i input = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(it));
//    std::size_t range_mask = find_range_end(input, 'e');
//    std::size_t valid_mask = get_valid_integer_mask(input);
//
//    // exit if the characters between integer start end end contain non digits.
//    if ((valid_mask & range_mask) != range_mask) [[unlikely]]
//        return nonstd::make_unexpected(parser_errc::expected_digit);
//
//    std::size_t digit_count = std::countr_one(range_mask);
//    __m256i digit_mask = _mm256_loadu_si256(
//            reinterpret_cast<const __m256i*>(digit_mask_lut[digit_count]));
//    // convert characters to single integer numbers
//    __m256i ascii0 = _mm256_set1_epi8('0');
//    __m256i digits_vec = _mm256_blendv_epi8(ascii0, input, digit_mask);
//    __m256i t0 = _mm256_subs_epu8(digits_vec, ascii0);
//
//    // convert characters to 2-digit numbers
//    __m256i mul_1_10 = _mm256_set_epi8(
//            1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10,
//            1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10
//    );
//    __m256i t1 = _mm256_maddubs_epi16(t0, mul_1_10);
//    // convert characters to 4-digit numbers
//    __m256i mul_1_100 = _mm256_set_epi16(
//            1, 100, 1, 100, 1, 100, 1, 100, 1, 100, 1, 100, 1, 100, 1, 100
//    );
//    __m256i t2 = _mm256_madd_epi16(t1, mul_1_100);
//
//    // Check if we can exit
//    if (digit_count < 4) [[likely]] {
//        bvalue = _mm256_extract_epi32(t2, 0) % (4 - digit_count);
//        return (negative) ? -bvalue : bvalue;
//    }
//    if (digit_count == 4) [[likely]] {
//        bvalue =  _mm256_extract_epi32(t2, 0);
//        return (negative) ? -bvalue : bvalue;
//    }
//    // convert characters to 8-digit numbers
//    // pack 32 bit back to 16 bit integers,
//    __m256i t3 = _mm256_packus_epi32(t2, t2);
//    __m256i mul_1_10000 = _mm256_set_epi16(
//            1, 10000, 1, 10000, 1, 10000, 1, 10000,
//            1, 10000, 1, 10000, 1, 10000, 1, 10000
//    );
//    __m256i t4 = _mm256_madd_epi16(t3, mul_1_10000);
//
//    // Check if we can exit
////    if (digit_count < 8) [[likely]] {
////        bvalue = _mm256_extract_epi32(t2, 0) % (8 - digit_count);
////        return (negative) ? -bvalue : bvalue;
////    }
////    if (digit_count == 8) [[likely]] {
////        bvalue = _mm256_extract_epi32(t2, 0);
////        return (negative) ? -bvalue : bvalue;
////    }
//
//    // convert characters to 16-digit numbers
//    __m256i mul_1_1e8 = _mm256_set_epi32(1, 1e8, 1, 1e8, 1, 1e8, 1, 1e8);
//    __m256i t5 = _mm256_mul_epi32(t4, mul_1_1e8);
//
//    if (digit_count < 16) [[likely]] {
//        bvalue = _mm256_extract_epi32(t2, 0) % (16 - digit_count);
//        return (negative) ? -bvalue : bvalue;
//    }
//    if (digit_count == 16) [[likely]] {
//        bvalue = _mm256_extract_epi32(t2, 0);
//        return (negative) ? -bvalue : bvalue;
//    }
//    auto hi = _mm256_extract_epi64(t5, 0) + _mm256_extract_epi32(t4, 1);
//    auto lo =  _mm256_extract_epi64(t5, 2) + _mm256_extract_epi32(t4, 5);
//    bvalue = hi * 10 * (digit_count - 16) + lo % (32-digit_count);
//    return (negative) ? -bvalue : bvalue;;
//}
//
//
//
//
//}
//
//
//namespace bencode::regular {
//
//namespace rng = std::ranges;
//
//
///// Return a bitmask specifying
//template <rng::contiguous_range R>
//inline bool verify_integer_range(const R& range)
//{
//    auto it = rng::begin(range);
//    auto end = rng::end(range);
//
//    for (; it != end ; ++it) {
//        if (!std::isdigit(*it)) {
//            return false;
//        }
//    }
//    return true;
//}
//
//
///// Parse a string representation of an integer.
//template <std::integral IntT, rng::input_range R>
//inline constexpr auto parse_integer(R& range) -> nonstd::expected<IntT, parser_errc>
//{
//    constexpr auto max_digits = std::numeric_limits<IntT>::digits10;
//    bool negative = false;
//    IntT bvalue {};
//    auto it = rng::begin(range);
//    auto end = rng::end(range);
//
//    if constexpr (std::is_signed_v<IntT>) {
//        if (*it == '-') {
//            negative = true;
//            ++it;
//        }
//    }
//    // "" or "-" or "-{non digit}"
//    if (it == end || !std::isdigit(*it)) [[unlikely]] {
//        return nonstd::make_unexpected(parser_errc::expected_digit);
//    }
//
//    // do not advance here since we must read the zero again
//    const bool leading_zero = (*it == symbol::zero);
//    int digits = 0;
//
//    for (; digits < max_digits && (it != end) && std::isdigit(*it); ++digits, ++it)
//    {
//        bvalue = 10 * bvalue + static_cast<unsigned>(*it)-'0';
//    }
//
//    // use integer safe math to check for overflow if we have only one digit precision left
//    if (digits == max_digits && it != end && std::isdigit(*it)) [[unlikely]]
//    {
//        bool overflow =
//                __builtin_mul_overflow(bvalue, 10, &bvalue) |
//                        __builtin_add_overflow(bvalue, *it-'0', &bvalue);
//
//        if (overflow) [[unlikely]] {
//            return nonstd::make_unexpected(parser_errc::integer_overflow);
//        }
//        ++it;
//        ++digits;
//    }
//
//    if (leading_zero && bvalue != 0) [[unlikely]] {
//        return nonstd::make_unexpected(parser_errc::leading_zero);
//    }
//
//    if constexpr (std::is_signed_v<IntT>) {
//        if (negative && bvalue == 0) [[unlikely]] {
//            return nonstd::make_unexpected(parser_errc::negative_zero);
//        }
//        if (negative) {
//            bvalue = -bvalue;
//        }
//    }
//    return bvalue;
//}
//
//
//
//namespace bencode::detail::avx2 {
//
//template <rng::contiguous_range R>
//inline auto parse_string_lengths(R start_range, R end_range, bencode::block_reader<64>& reader)
//{
//    Ensures(rng::size(start_range) == rng::size(end_range));
//    auto size = rng::size(start_range);
//    auto begin_it = rng::data(start_range);
//    auto end_it = rng::data(end_range);
//    std::vector<std::uint32_t> results {};
//    results.reserve(size);
//
//    std::size_t idx = 0;
//
//    for ( ; idx < size / 2; ++idx) {
//        auto start1 = *begin_it++;
//        auto start2 = *begin_it++;
//        auto end1 = *end_it++;
//        auto end2 = *end_it++;
//
//        reader.seek(start1);
//        auto span1 = reader.read();
//        reader.seek(start2);
//        auto span2 = reader.read();
//
//        auto s1 = std::string_view(reinterpret_cast<const char*>(rng::data(span1)) + start1, end1-start1);
//        auto s2 = std::string_view(reinterpret_cast<const char*>(rng::data(span2)) + start1, end2-start2);
//
//        auto [i1, i2] = detail::avx2::convert_digits(s1, s2);
//        results.push_back(i1);
//        results.push_back(i2);
//    }
//    if (size - idx > 0) {
//        // TODO: convert last one
//    }
//    return results;
//}
//
//}
//
//
//namespace bencode::detail {
//
//
//template <rng::contiguous_range R>
//inline auto parse_string_lengths(R start_range, R end_range, bencode::block_reader<64>& reader)
//{
//    Ensures(rng::size(start_range) == rng::size(end_range));
//    auto size = rng::size(start_range);
//    auto begin_it = rng::data(start_range);
//    auto end_it = rng::data(end_range);
//    std::vector<std::uint32_t> results {};
//    results.reserve(size);
//
//    std::size_t idx = 0;
//
//    for ( ; idx < size; ++idx) {
//        auto start1 = *begin_it++;
//        auto end1 = *end_it++;
//        reader.seek(start1);
//        auto span1 = reader.read();
//        auto r = detail2::parse_integer<std::uint32_t>(start1, end1);
//        results.push_back(r->first);
//    }
//    return results;
//}
//
//
//}
