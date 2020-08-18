#pragma once

#include <cstdint>
#include <string_view>
#include <ranges>
#include <immintrin.h>

#include <nonstd/expected.hpp>
#include "bencode/detail/parser/common.hpp"
#include "bencode/detail/utils.hpp"

static constexpr unsigned char digit_mask_lut[20][32] = {
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

static constexpr std::array<std::uint64_t,  19> power_of_10 = {
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


namespace bencode::detail::avx2 {

/// Parse an integer
template <typename It>
    requires std::contiguous_iterator<It>
nonstd::expected<std::int64_t, parser_errc> parse_integer(It& begin, It end)
{
    Expects(std::distance(begin, end) >= 32);

    const char* it = &(*begin);

    // load 32 characters at once
    const __m256i input = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(it));
    bool is_negative = _mm256_extract_epi8(input, 0) == '-';

    // load constants
    const __m256i zero       = _mm256_setzero_si256();
    const __m256i ascii0     = _mm256_set1_epi8('0');

    // Convert ascii code points '0'-'9' [48-57] to the range [117-127] to check for digits
    // with a single comparison
    __m256i td = _mm256_add_epi8(input, _mm256_set1_epi8(70));
    __m256i digits_mask = _mm256_cmpgt_epi8(td, _mm256_set1_epi8(117));
    std::uint32_t digits_bitmask =  _mm256_movemask_epi8(digits_mask);

    std::size_t digit_count = std::countr_one(digits_bitmask);
    auto value_end = begin + is_negative + digit_count;

    __m256i blend_mask = _mm256_loadu_si256(
            reinterpret_cast<const __m256i*>(digit_mask_lut[digit_count]));
    __m256i blended = _mm256_blendv_epi8(ascii0, input, blend_mask);
    __m256i digits = _mm256_undefined_si256();

    // reverse digits: most significant digit must be the most significant in the vector as well.
    if (digit_count > 16) [[unlikely]] {
        __m256i reverse_mask = _mm256_set_epi8(
                0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
                0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);

        // swap chars per 128 bit lane
        __m256i half_reversed = _mm256_shuffle_epi8(input, reverse_mask);
        // swap two 128 bit lanes
        digits = _mm256_permute2x128_si256(half_reversed, half_reversed, 0x21);
    } else {
        __m256i shuffle_mask = _mm256_loadu2_m128i(
                reinterpret_cast<const __m128i*>(digit_mask_lut[0]),
                reinterpret_cast<const __m128i*>(digit_mask_lut[digit_count]));
        digits = _mm256_shuffle_epi8(blended, shuffle_mask);
    }

    // fill element out of range of the span with zeros.
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

    std::int64_t r;

    if (digit_count <= 8) [[likely]] {
        r = _mm256_extract_epi64(t4, 0);
    }
    else {
        std::uint64_t lo1 = _mm256_extract_epi32(t4, 0);
        std::uint64_t lo2 = _mm256_extract_epi32(t4, 1);

        if (digit_count <= 16) [[likely]] {
            r =  lo2 * 100000000 + lo1;
        }
        else {
            std::uint64_t hi1 = _mm256_extract_epi32(t4, 4);
            std::uint64_t hi2 = _mm256_extract_epi32(t4, 5);

            auto r1 = (hi2 * 100000000 + hi1) * power_of_10[digit_count - 16];
            auto r2 = lo2 / power_of_10[24 - digit_count];
            r = r1 + r2;
        }
    }
    if (is_negative && r == 0) [[unlikely]] {
        return nonstd::make_unexpected(parser_errc::negative_zero);
    }

    begin = value_end;
    return is_negative ? -r : r;
}

/// @copydoc bencode::detail::parse_integer()
template <typename It>
    requires std::input_iterator<It>
constexpr nonstd::expected<std::int64_t, parser_errc> parse_integer(It&& begin, It end) noexcept
{
    auto b = begin;
    return parse_integer(b, end);
}

/// Decode a bencoded integer.
///
/// Try to decode one integer token (eg. i666e) from a char sequence
/// given by a pair of iterators.
///
/// @param first Start of the char sequence.
/// @param last End of the char sequence.
/// @param ec Error code in case of parsing errors.
/// @return Returns an optional pair of the bvalue and an iterator pointing to the
///         first non decoded character in the input sequence.
///         When the optional is empty a parsing error occurred.
template<std::contiguous_iterator It>
constexpr auto bdecode_integer(It& first, It last) noexcept
-> nonstd::expected<std::int64_t, parser_errc>
{
    Expects(std::distance(first, last) >= 32);
    const char* it = &(*first);

    if (it == last) [[unlikely]] {
        return nonstd::make_unexpected(parser_errc::unexpected_eof);
    }
    if (*it != 'i') [[unlikely]] {
        return nonstd::make_unexpected(parser_errc::expected_integer_start_token);
    }
    // skip 'i'
    ++it;
    auto result = parse_integer(it, last);

    // pass possible errors from parse_integer
    if (!result) [[unlikely]] {
        return nonstd::make_unexpected(result.error());
    }
    const auto value = *result;

    // verify the integer is correctly terminated with the "e"
    if (it == last || *it != symbol::end) [[unlikely]] {
        return nonstd::make_unexpected(parser_errc::expected_end);
    }

    ++it;
    first = it;
    return value;
}

};