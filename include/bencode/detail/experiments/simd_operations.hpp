//
// Created by fbdtemme on 4/25/20.
//
#pragma once
#include <iterator>
#include <concepts>
#include <cstdint>
#include <bit>
#include <gsl/gsl_assert>

#include <immintrin.h>
#include "common.hpp"

#include "tables.hpp"

namespace bencode::detail {

namespace rng = std::ranges;


constexpr std::uint32_t bit_swap(std::uint32_t v) noexcept
{
    int reverse = bit_reverse_lut[v & 0xff] << 24         |  // consider first 8 bits
                  bit_reverse_lut[(v >>  8) & 0xff] << 16 |	 // consider next 8 bits
                  bit_reverse_lut[(v >> 16) & 0xff] << 8  | // consider next 8 bits
                  bit_reverse_lut[(v >> 24) & 0xff];   		// consider last 8 bits

    return reverse;
}

constexpr std::uint64_t bit_swap(std::uint64_t v) noexcept
{
    auto reverse_lo = bit_swap(std::uint32_t(v));
    auto reverse_hi = bit_swap(std::uint32_t(v >> 32));
    auto result = std::uint64_t(reverse_lo) << 32 | std::uint64_t(reverse_hi);
    return result;
}

} // namespace bencode::detail


namespace bencode::detail::avx2
{

enum class token_class : std::uint8_t
{
    digit = 1,
    minus = 2,
    semicolon = 4,
    integer_begin = 8,
    list_begin = 16,
    dict_begin = 32,
    end = 64
};


// Classify the character group of each character in a 256 bit vector.
// Return a vector with each bvalue specifying the character class.
//
// Character classes:
// +--------+------+-------+
// | Hex    | Char | Value |
// +--------+------+-------+
// | 30..39 | 0..9 | 1     |
// | 2D     | -    | 2     |
// | 3A     | :    | 4     |
// | 6C     | i    | 8     |
// | 69     | l    | 16    |
// | 64     | d    | 32    |
// | 65     | e    | 64    |
// +--------+------+-------+
//
// Lookup table:
// +-------------+------------+---+---+---+---+----+----+---+---+---+---+---+---+----+---+---+---+
// |             | low nibble | 0 | 1 | 2 | 3 | 4  | 5  | 6 | 7 | 8 | 9 | A | B | C  | D | E | F |
// +-------------+------------+---+---+---+---+----+----+---+---+---+---+---+---+----+---+---+---+
// | high nibble |            | 1 | 1 | 1 | 1 | 33 | 65 | 1 | 1 | 1 | 9 | 4 | 0 | 16 | 2 | 0 | 0 |
// +-------------+------------+---+---+---+---+----+----+---+---+---+---+---+---+----+---+---+---+
// |           0 |          0 |   |   |   |   |    |    |   |   |   |   |   |   |    |   |   |   |
// +-------------+------------+---+---+---+---+----+----+---+---+---+---+---+---+----+---+---+---+
// |           1 |          0 |   |   |   |   |    |    |   |   |   |   |   |   |    |   |   |   |
// +-------------+------------+---+---+---+---+----+----+---+---+---+---+---+---+----+---+---+---+
// |           2 |          2 |   |   |   |   |    |    |   |   |   |   |   |   |    | 2 |   |   |
// +-------------+------------+---+---+---+---+----+----+---+---+---+---+---+---+----+---+---+---+
// |           3 |          5 | 1 | 1 | 1 | 1 | 1  | 1  | 1 | 1 | 1 | 1 | 4 |   |    |   |   |   |
// +-------------+------------+---+---+---+---+----+----+---+---+---+---+---+---+----+---+---+---+
// |           4 |          0 |   |   |   |   |    |    |   |   |   |   |   |   |    |   |   |   |
// +-------------+------------+---+---+---+---+----+----+---+---+---+---+---+---+----+---+---+---+
// |           5 |          0 |   |   |   |   |    |    |   |   |   |   |   |   |    |   |   |   |
// +-------------+------------+---+---+---+---+----+----+---+---+---+---+---+---+----+---+---+---+
// |           6 |        120 |   |   |   |   | 32 | 64 |   |   |   | 8 |   |   | 16 |   |   |   |
// +-------------+------------+---+---+---+---+----+----+---+---+---+---+---+---+----+---+---+---+
// |           7 |          0 |   |   |   |   |    |    |   |   |   |   |   |   |    |   |   |   |
// +-------------+------------+---+---+---+---+----+----+---+---+---+---+---+---+----+---+---+---+
inline __m256i classify_characters(__m256i input) noexcept
{
    const auto low_lut = _mm256_setr_epi8(
            1, 1, 1, 1, 33, 65, 1, 1, 1, 9, 4, 0, 16, 2, 0, 0,
            1, 1, 1, 1, 33, 65, 1, 1, 1, 9, 4, 0, 16, 2, 0, 0);
    const auto high_lut = _mm256_setr_epi8(
            0, 0, 2, 5, 0, 0, 120, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 2, 5, 0, 0, 120, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    auto low_nibble_mask = _mm256_set1_epi8(0xF);
    auto low_nibbles = _mm256_and_si256(input, low_nibble_mask);
    // shift high nibbles to low nibbles. mask away the carry.
    auto high_nibbles = _mm256_and_si256(_mm256_srli_epi16(input, 4), low_nibble_mask);
    // first lookup of the low nibbles
    auto shuf_lo = _mm256_shuffle_epi8(low_lut, low_nibbles);
    // second lookup of the high nibbles
    auto shuf_hi = _mm256_shuffle_epi8(high_lut, high_nibbles);
    // AND low and high to get the character class values.
    auto result = _mm256_and_si256(shuf_lo, shuf_hi);
    return result;
}


/// Reverse a string in a 256-bit integer vector.
inline __m256i reverse(__m256i input) noexcept
{
    __m256i reverse_mask = _mm256_set_epi8(
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);

    // swap chars per 128 bit lane
    __m256i half_reversed = _mm256_shuffle_epi8(input, reverse_mask);
    // swap two 128 bit lanes
    __m256i result = _mm256_permute2x128_si256(half_reversed, half_reversed, 0x21);
    return result;
}


/// Return the bit-reverse of packed 32-bit unsigned integers.
inline __m256i bit_swap32(__m256i input)
{
    __m256i low_nibble_mask = _mm256_set1_epi8(0x0f);
    __m256i shuffle_index = _mm256_setr_epi8(
            3, 2, 1, 0, 7, 6, 5, 4, 11, 10, 9, 8, 15, 14, 13, 12,
            3, 2, 1, 0, 7, 6, 5, 4, 11, 10, 9, 8, 15, 14, 13, 12
    );
    __m256i high_lut = _mm256_setr_epi8(
            0x00, 0x08, 0x04, 0x0c, 0x02, 0x0a, 0x06, 0x0e, 0x01, 0x09, 0x05, 0x0d, 0x03, 0x0b, 0x07, 0x0f,
            0x00, 0x08, 0x04, 0x0c, 0x02, 0x0a, 0x06, 0x0e, 0x01, 0x09, 0x05, 0x0d, 0x03, 0x0b, 0x07, 0x0
    );
    __m256i low_lut = _mm256_setr_epi8(
            0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0, 0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
            0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0, 0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0
    );
    // reverse the bytes in every 32 bit word
    auto byte_reverse = _mm256_shuffle_epi8(input, shuffle_index);
    auto low_nibbles = _mm256_and_si256(byte_reverse, low_nibble_mask);
    // shift high nibbles to low nibbles. mask away the carry.
    auto high_nibbles = _mm256_and_si256(_mm256_srli_epi16(byte_reverse, 4), low_nibble_mask);
    // first lookup of the low nibbles
    auto shuf_lo = _mm256_shuffle_epi8(low_lut, low_nibbles);
    // second lookup of the high nibbles
    auto shuf_hi = _mm256_shuffle_epi8(high_lut, high_nibbles);
    // OR low and high to combine the result
    auto result = _mm256_or_si256(shuf_lo, shuf_hi);
    return result;
}

/// Return the bit reverse of packed 64-bit unsigned integers.
inline __m256i bit_swap64(__m256i input)
{
    __m256i low_nibble_mask = _mm256_set1_epi8(0x0f);
    __m256i shuffle_index = _mm256_setr_epi8(
            7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8,
            7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8
    );
    __m256i high_lut = _mm256_setr_epi8(
            0x00, 0x08, 0x04, 0x0c, 0x02, 0x0a, 0x06, 0x0e, 0x01, 0x09, 0x05, 0x0d, 0x03, 0x0b, 0x07, 0x0f,
            0x00, 0x08, 0x04, 0x0c, 0x02, 0x0a, 0x06, 0x0e, 0x01, 0x09, 0x05, 0x0d, 0x03, 0x0b, 0x07, 0x0
    );
    __m256i low_lut = _mm256_setr_epi8(
            0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0, 0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
            0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0, 0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0
    );
    // reverse the bytes in every 64 bit word
    auto byte_reverse = _mm256_shuffle_epi8(input, shuffle_index);
    auto low_nibbles = _mm256_and_si256(byte_reverse, low_nibble_mask);
    // shift high nibbles to low nibbles. mask away the carry.
    auto high_nibbles = _mm256_and_si256(_mm256_srli_epi16(byte_reverse, 4), low_nibble_mask);
    // first lookup of the low nibbles
    auto shuf_lo = _mm256_shuffle_epi8(low_lut, low_nibbles);
    // second lookup of the high nibbles
    auto shuf_hi = _mm256_shuffle_epi8(high_lut, high_nibbles);
    // OR low and high to combine the result
    auto result = _mm256_or_si256(shuf_lo, shuf_hi);
    return result;
}

/// Return the position of digits from a character classification vector.
inline std::uint32_t get_digits_mask(__m256i classified_input) noexcept
{
    // Convert ascii code points '0'-'9' [48-57] to the range [117-127] to check for digits
    // with a single comparison
    __m256i digit_mask = _mm256_set1_epi8(static_cast<std::int8_t>(token_class::digit));
    __m256i result = _mm256_cmpeq_epi8(classified_input, digit_mask);
    return _mm256_movemask_epi8(result);
}

/// Return the position of digits from a character classification vector.
inline std::uint32_t get_minus_mask(__m256i classified_input) noexcept
{
    // Convert ascii code points '0'-'9' [48-57] to the range [117-127] to check for digits
    // with a single comparison
    __m256i minus_mask = _mm256_set1_epi8(static_cast<std::int8_t>(token_class::minus));
    __m256i result = _mm256_cmpeq_epi8(classified_input, minus_mask);
    return _mm256_movemask_epi8(result);
}

/// Return the position of digits and minus symbol from a character classification vector.
inline std::uint32_t get_integer_symbols_mask(__m256i classified_input) noexcept
{
    // Convert ascii code points '0'-'9' [48-57] to the range [117-127] to check for digits
    // with a single comparison
    auto digits_mask = get_digits_mask(classified_input);
    auto minus_mask = get_minus_mask(classified_input);
    auto result =  digits_mask | minus_mask;
    return result;
}

/// Return the position of digits and minus symbol.
inline std::uint32_t get_integer_symbols_mask(__m256i digits_mask, __m256i minus_mask) noexcept
{
    __m256i result = _mm256_or_si256(digits_mask, minus_mask);
    return _mm256_movemask_epi8(result);
}

/// Return the position of integer start tokens 'i'.
inline std::uint32_t get_integer_begin_mask(__m256i classified_input) noexcept
{
    __m256i i_mask = _mm256_set1_epi8(static_cast<std::int8_t>(token_class::integer_begin));
    __m256i result = _mm256_cmpeq_epi8(classified_input, i_mask);
    return _mm256_movemask_epi8(result);
}

/// Return the position of integer start tokens 'i'.
inline std::uint32_t get_list_start_mask(__m256i classified_input) noexcept
{
    __m256i l_mask = _mm256_set1_epi8(static_cast<std::int8_t>(token_class::list_begin));
    __m256i result = _mm256_cmpeq_epi8(classified_input, l_mask);
    return _mm256_movemask_epi8(result);
}

/// Return the position of integer start tokens 'i'.
inline std::uint32_t get_dict_start_mask(__m256i classified_input) noexcept
{
    __m256i d_mask = _mm256_set1_epi8(static_cast<std::int8_t>(token_class::dict_begin));
    __m256i result = _mm256_cmpeq_epi8(classified_input, d_mask);
    return _mm256_movemask_epi8(result);
}

/// Return the position of integer start tokens 'i'.
inline std::uint32_t get_end_symbol_mask(__m256i classified_input) noexcept
{
    __m256i e_mask = _mm256_set1_epi8(static_cast<std::int8_t>(token_class::end));
    __m256i result = _mm256_cmpeq_epi8(classified_input, e_mask);
    return _mm256_movemask_epi8(result);
}

/// Return the position of integer start tokens 'i'.
inline std::uint32_t get_semicolon_mask(__m256i classified_input) noexcept
{
    __m256i semicolon_mask = _mm256_set1_epi8(static_cast<std::int8_t>(token_class::semicolon));
    __m256i result = _mm256_cmpeq_epi8(classified_input, semicolon_mask);
    return _mm256_movemask_epi8(result);
}

inline __m256i convert_3digits(__m256i input)
{
    // 128 bit lane:
    // t0 = [ 0 | 5 | 7 | 9 | 0 | 1 | 2 | 3 | 0 | 9 | 2 | 5 | 0 | 3 | 2 | 7 ]
    // t1 = [  500  |  79   |  100  |   23  |   900 |   25  |  300  |  27   ]
    // t2 = [  579  |  123  |  925  | 327   |  579  |  123  |  925  | 327   ]
    __m256i ascii0  = _mm256_set1_epi8('0');
    __m256i t0 = _mm256_subs_epu8(input, ascii0);
    __m256i mul_all = _mm256_set_epi8(
            0, 100, 10, 1, 0, 100, 10, 1, 0, 100, 10, 1, 0, 100, 10, 1,
            0, 100, 10, 1, 0, 100, 10, 1, 0, 100, 10, 1, 0, 100, 10, 1);
    __m256i t1 = _mm256_maddubs_epi16(t0, mul_all);
    __m256i t2 = _mm256_hadd_epi16(t1, t1);
    return t2;
}

// Convert 16 4-digit decimal strings to integers.
inline __m256i convert_4digits(__m256i input)
{
    // 128 bit lane:
    // t0 = [ 0 | 1 | 0 | 2 | 0 | 3 | 0 | 4 | 0 | 5 | 0 | 6 | 0 | 7 | 0 | 8 ]
    // t0 = [     1 |     2 |     3 |     4 |     5 |     6 |     7 |     8 ]
    //    * [  1000 |   100 |    10 |     1 |  1000 |   100 |    10 |     1 ]
    // t1 = [     1200     |        34      |       5600    |       78      ]
    // t2 = [     1234     |       5678     |       1234    |      5678     ]

    __m256i ascii0  = _mm256_set1_epi8('0');
    __m256i t0 = _mm256_subs_epu8(input, ascii0);
    __m256i mul_all = _mm256_setr_epi16(
            1000, 100, 10, 1, 1000, 100, 10, 1, 1000, 100, 10, 1, 1000, 100, 10, 1);
    __m256i t1 = _mm256_maddubs_epi16(t0, mul_all);
    __m256i t2 = _mm256_hadd_epi32(t1, t1);
    return t2;
}

// Convert 4 8-digit decimal strings to integers.
inline __m256i convert_8digits(__m256i input)
{
    __m256i ascii0  = _mm256_set1_epi8('0');
    __m256i t0 = _mm256_subs_epu8(input, ascii0);
    __m256i mul_1_10 = _mm256_set_epi8(
            10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1,
            10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1);
    // 2 digit
    __m256i t1 = _mm256_maddubs_epi16(t0, mul_1_10);
    // 4-digit numbers
    __m256i mul_1_100  = _mm256_set_epi16(
            100, 1, 100, 1, 100, 1, 100, 1, 100, 1, 100, 1, 100, 1, 100, 1);
    __m256i t2 = _mm256_madd_epi16(t1, mul_1_100);
    // convert from 32-bit into 16-bit element vector
    __m256i t3 = _mm256_packus_epi32(t2, t2);
    // convert to 8-digit numbers
    __m256i mul_1_10000 = _mm256_set_epi16(
            10000, 1, 10000, 1, 10000, 1, 10000, 1, 10000, 1, 10000, 1, 10000, 1, 10000, 1);
    __m256i t4 = _mm256_madd_epi16(t3, mul_1_10000);
    return t4;
}



// Convert two 16 digit decimal strings to integers.
inline __m256i convert_16digits(__m256i input)
{
    // 128 bit lane:
    // t0 = [     1 |     2 |     3 |     4 |     5 |     6 |     7 |     8 ]
    //    * [  1000 |   100 |    10 |     1 |  1000 |   100 |    10 |     1 ]
    // t1 = [     1200     |        34      |       5600    |       78      ]
    // t4 = [          12345678             |            12345678           ]
    // t5 = [                       1234567812345678                        ]

    __m256i ascii0  = _mm256_set1_epi8('0');
    __m256i t0 = _mm256_subs_epu8(input, ascii0);

    // convert characters to 2-digit numbers
    __m256i mul_1_10 = _mm256_set_epi8(
            10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1,
            10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1);

    __m256i t1 = _mm256_maddubs_epi16(t0, mul_1_10);
    // convert characters to 4-digit numbers
    __m256i mul_1_100 = _mm256_set_epi16(
           100, 1, 100, 1, 100, 1, 100, 1, 100, 1, 100, 1, 100, 1, 100, 1);

    __m256i t2 = _mm256_madd_epi16(t1, mul_1_100);

    // convert characters to 8-digit numbers
    // pack 32 bit back to 16 bit integers,
    __m256i t3 = _mm256_packus_epi32(t2, t2);
    __m256i mul_1_10000 = _mm256_set_epi16(
            10000, 1, 10000, 1, 10000, 1, 10000, 1,
            10000, 1, 10000, 1, 10000, 1, 10000, 1 );

    __m256i t4 = _mm256_madd_epi16(t3, mul_1_10000);

    // convert characters to 16-digit numbers
    __m256i mul_1_1e8 = _mm256_set_epi32(
            100000000, 1, 100000000, 1, 100000000, 1, 100000000, 1);
    __m256i t5 = _mm256_mul_epi32(t4, mul_1_1e8);
    return t5;
}


/// Convert 64-bit integers
inline auto convert_digits(std::string_view s1) -> std::uint64_t
{
    Expects(std::size(s1) >= 32);
    const char* it = rng::data(s1);
    std::size_t digit_count = s1.size();
    // load 32 characters
    __m256i ascii0 = _mm256_set1_epi8('0');
    __m256i zero = _mm256_setzero_si256();

    __m256i input = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(it));
    __m256i blend_mask = _mm256_loadu_si256(
            reinterpret_cast<const __m256i*>(mask_lut[digit_count]));
    __m256i blended = _mm256_blendv_epi8(ascii0, input, blend_mask);
    __m256i digits = _mm256_undefined_si256();

    if (s1.size() > 16) [[unlikely]] {
        digits = reverse(blended);
    } else {
        __m256i shuffle_mask = _mm256_loadu2_m128i(
                reinterpret_cast<const __m128i*>(integer_16d_shuffle_lut[0]),
                reinterpret_cast<const __m128i*>(integer_16d_shuffle_lut[digit_count]));
        digits = _mm256_shuffle_epi8(blended, shuffle_mask);
    }

    // fill element out of range of the span with zeros.
    // reverse digits: most significant digit must be the most significant in the vector as well.
    __m256i t0 = _mm256_subs_epu8(digits, ascii0);

    // convert characters to 2-digit numbers
    __m256i mul_1_10 = _mm256_set_epi8(
            10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1,
            10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1);

     __m256i t1 = _mm256_maddubs_epi16(t0, mul_1_10);
    // convert characters to 4-digit numbers
    __m256i mul_1_100 = _mm256_set_epi16(
            100, 1, 100, 1, 100, 1, 100, 1, 100, 1, 100, 1, 100, 1, 100, 1);

     __m256i t2 = _mm256_madd_epi16(t1, mul_1_100);

    // convert characters to 8-digit numbers
    // pack 32 bit back to 16 bit integers,
     __m256i t3 = _mm256_packus_epi32(t2, t2);
     __m256i mul_1_10000 = _mm256_set_epi16(
            10000, 1, 10000, 1, 10000, 1, 10000, 1,
            10000, 1, 10000, 1, 10000, 1, 10000, 1 );

     __m256i t4 = _mm256_madd_epi16(t3, mul_1_10000);

    if (digit_count <= 8) [[likely]] {
        auto r = _mm256_extract_epi64(t4, 0);
        return r;
    }

    std::uint64_t lo1 = _mm256_extract_epi32(t4, 0);
    std::uint64_t lo2 = _mm256_extract_epi32(t4, 1);

    if (digit_count <= 16) [[likely]] {
        auto r =  lo2 * 100000000 + lo1;
        return r;
    }
    if (digit_count > 20) [[unlikely]] {
        throw std::overflow_error("overflow");
    }

    std::uint64_t hi1 = _mm256_extract_epi32(t4, 4);
    std::uint64_t hi2 = _mm256_extract_epi32(t4, 5);

    auto r1 = (hi2 * 100000000 + hi1) * power_of_10[digit_count - 16];
    auto r2 = lo2 / power_of_10[24 - digit_count];
    auto r = r1 + r2;
    return r;
}



inline auto convert_digits_serial(std::string_view s1) -> std::uint64_t
{
    std::uint64_t value = 0;
    auto it = rng::begin(s1);
    auto end = rng::end(s1);
    std::size_t digits = rng::size(s1);
    constexpr auto max_digits = std::numeric_limits<std::size_t>::digits10;

    for (std::size_t i = 0; i < max_digits && it != end; ++i, ++it) {
        value = 10 * value + static_cast<unsigned>(*it)-'0';
    }

    // use integer safe math to check for overflow if we have only one digit precision left
    if (digits == max_digits && it != end && std::isdigit(*it)) [[unlikely]] {
        bool overflow =
                __builtin_mul_overflow(value, 10, &value) |
                __builtin_add_overflow(value, *it-'0', &value);
    }
    return value;
}



/// Convert two decimal string of maximum 16 digits to 32-bit integers.
inline auto convert_digits(std::string_view s1, std::string_view s2)
    -> std::pair<std::uint32_t, std::uint32_t>
{
    __m256i ascii0 = _mm256_set1_epi8('0');
    __m256i input = _mm256_loadu2_m128i(
            reinterpret_cast<const __m128i*>(s2.data()),
            reinterpret_cast<const __m128i*>(s1.data()));
    __m256i blend_mask = _mm256_loadu2_m128i(
            reinterpret_cast<const __m128i*>(mask_lut[s2.size()]),
            reinterpret_cast<const __m128i*>(mask_lut[s1.size()]));
    __m256i shuffle_mask = _mm256_loadu2_m128i(
            reinterpret_cast<const __m128i*>(integer_16d_shuffle_lut[s2.size()]),
            reinterpret_cast<const __m128i*>(integer_16d_shuffle_lut[s1.size()]));

    // fill element out of range of the span with zeros.
    __m256i blended = _mm256_blendv_epi8(ascii0, input, blend_mask);
    // reverse digits: most significant digit must be the most significant in the vector as well.
    __m256i digits  = _mm256_shuffle_epi8(blended, shuffle_mask);
    __m256i result = convert_16digits(digits);
    std::uint32_t i1 = _mm256_extract_epi64(result, 0);
    std::uint32_t i2 = _mm256_extract_epi64(result, 2);
    return {i1, i2};
}

inline auto convert_digits(std::string_view s1, std::string_view s2,
                    std::string_view s3, std::string_view s4)
        -> std::array<std::uint32_t, 4>
{
    __m256i ascii0 = _mm256_set1_epi8('0');
    __m256i input_lo = _mm256_loadu2_m128i(
            reinterpret_cast<const __m128i*>(s2.data()),
            reinterpret_cast<const __m128i*>(s1.data()));
    __m256i input_hi = _mm256_loadu2_m128i(
            reinterpret_cast<const __m128i*>(s4.data()),
            reinterpret_cast<const __m128i*>(s3.data()));
    __m256i blend_mask_lo = _mm256_loadu2_m128i(
            reinterpret_cast<const __m128i*>(mask_lut[s2.size()]),
            reinterpret_cast<const __m128i*>(mask_lut[s1.size()]));
    __m256i blend_mask_hi = _mm256_loadu2_m128i(
            reinterpret_cast<const __m128i*>(mask_lut[s4.size()]),
            reinterpret_cast<const __m128i*>(mask_lut[s3.size()]));
    __m256i shuffle_mask_lo = _mm256_loadu2_m128i(
            reinterpret_cast<const __m128i*>(integer_16d_shuffle_lut[s2.size()]),
            reinterpret_cast<const __m128i*>(integer_16d_shuffle_lut[s1.size()]));
    __m256i shuffle_mask_hi = _mm256_loadu2_m128i(
            reinterpret_cast<const __m128i*>(integer_16d_shuffle_lut[s4.size()]),
            reinterpret_cast<const __m128i*>(integer_16d_shuffle_lut[s3.size()]));
    // fill element out of range of the span with zeros.
    __m256i blended_lo = _mm256_blendv_epi8(ascii0, input_lo, blend_mask_lo);
    __m256i blended_hi = _mm256_blendv_epi8(ascii0, input_hi, blend_mask_hi);
    // reverse digits: most significant digit must be the most significant in the vector as well.
    __m256i digits_lo = _mm256_shuffle_epi8(blended_lo, shuffle_mask_lo);
    __m256i digits_hi = _mm256_shuffle_epi8(blended_hi, shuffle_mask_hi);
    // move the two 64 bit values to the high bvalue in the 128 bit lanes
//    __m256i digits_hi_shift = _mm256_permute4x64_epi64(digits_hi, 0b10110001);
    // combine in a single vector
    __m256i digits = _mm256_blend_epi32(digits_hi, digits_lo, 0b00110011);
    __m256i result = convert_8digits(digits);

//    char i0 = _mm256_extract_epi8(digits, 0);
//    char i1 = _mm256_extract_epi8(digits, 1);
//    char i2 = _mm256_extract_epi8(digits, 2);
//    char i3 = _mm256_extract_epi8(digits, 3);
//    char i4 = _mm256_extract_epi8(digits, 4);
//    char i5 = _mm256_extract_epi8(digits, 5);
//    char i6 = _mm256_extract_epi8(digits, 6);
//    char i7 = _mm256_extract_epi8(digits, 7);
//    char i8 = _mm256_extract_epi8(digits, 8);
//    char i9 = _mm256_extract_epi8(digits, 9);
//    char i10 = _mm256_extract_epi8(digits, 10);
//    char i11 = _mm256_extract_epi8(digits, 11);
//    char i12 = _mm256_extract_epi8(digits, 12);
//    char i13 = _mm256_extract_epi8(digits, 13);
//    char i14 = _mm256_extract_epi8(digits, 14);
//    char i15 = _mm256_extract_epi8(digits, 15);
//    char i_16 = _mm256_extract_epi8(digits,  16);
//    char i_17 = _mm256_extract_epi8(digits,  17);
//    char i_18 = _mm256_extract_epi8(digits,  18);
//    char i_19 = _mm256_extract_epi8(digits,  19);
//    char i_20 = _mm256_extract_epi8(digits,  20);
//    char i_21 = _mm256_extract_epi8(digits,  21);
//    char i_22 = _mm256_extract_epi8(digits,  22);
//    char i_23 = _mm256_extract_epi8(digits,  23);
//    char i_24 = _mm256_extract_epi8(digits,  24);
//    char i_25 = _mm256_extract_epi8(digits,  25);
//    char i_260 = _mm256_extract_epi8(digits, 26);
//    char i_271 = _mm256_extract_epi8(digits, 27);
//    char i_282 = _mm256_extract_epi8(digits, 28);
//    char i_293 = _mm256_extract_epi8(digits, 29);
//    char i_304 = _mm256_extract_epi8(digits, 30);
//    char i_315 = _mm256_extract_epi8(digits, 31);

    std::uint32_t i0 = _mm256_extract_epi32(result, 0);
    std::uint32_t i1 = _mm256_extract_epi32(result, 1);
    std::uint32_t i2 = _mm256_extract_epi32(result, 4);
    std::uint32_t i3 = _mm256_extract_epi32(result, 5);
    return {i0, i2, i1, i3};
}
//    return {i1, i2};





} // namespace bencode::detail::avx

namespace bencode::detail {


template <std::integral T>
constexpr auto clear_lowest_bit(T value) noexcept -> T
{ return value & (value-1); }

/// Combine two 32 bit unsigned integers to a 64 bit result.
constexpr auto combine_bits(std::uint32_t hi, std::uint32_t lo) noexcept -> std::uint64_t
{ return (std::uint64_t(hi) << 32) | std::uint64_t(lo); }

constexpr auto high_bits(std::uint64_t v) noexcept -> std::uint32_t
{ return v >> 32; }

constexpr auto low_bits(std::uint64_t v) noexcept -> std::uint32_t
{ return v & 0xFFFFFFFF; }


inline auto extract_indices_from_bitmask(
        std::uint64_t bitmask,
        std::size_t offset,
        std::vector<std::size_t>& out) -> std::size_t
{
    if (bitmask == 0) [[unlikely]] return 0;

    auto previous_size = out.size();
    auto it = std::back_inserter(out);
    auto bit_count = std::popcount(bitmask);
    auto index = 0;

    // Process 8 together to limit branches.
    for (std::size_t i = 0; i < 8 ; ++i) {
        index = std::countr_zero(bitmask);
        bitmask = clear_lowest_bit(bitmask);
        *it++ = index + offset;
    }

    // More then 8 indices is unlikely.
    if (bit_count > 8) [[unlikely]] {
        for (std::size_t i = 0; i < 8; ++i) {
            index = std::countr_zero(bitmask);
            bitmask = clear_lowest_bit(bitmask);
            *it++ = index + offset;
        }
        // More then 16 indices. Proceed one by one since this is highly unlikely.
        if (bit_count > 16) [[unlikely]] {
            while (bitmask) {
                index = std::countr_zero(bitmask);
                bitmask = clear_lowest_bit(bitmask);
                *it++ = index + offset;
            }
        }
    }

    // remove garbage values by repositioning the head pointer of the vector.
    out.resize(previous_size + bit_count);
    return bit_count;
}










} // namespace bencode::detail