#pragma once
#include <string_view>
#include <span>
#include <bit>
#include <ranges>
#include <immintrin.h>

#include "../simd_operations.hpp"
#include "block_reader.hpp"

static_assert( -1 >> 1 == -1, "unsupported platform: signed right shift must be of arithmetic");

//namespace stdx = std::experimental;

namespace bencode {


namespace rng = std::ranges;


struct token_indexer
{
    struct string_index_entry {
        std::size_t start;
        std::size_t offset;
        std::size_t size;
    };

    struct integer_index_entry {
        std::size_t start;
        std::size_t end;
        std::int64_t value;
    };

    token_indexer() = default;

    auto integer_begin() const -> const std::vector<std::size_t > &
    { return integer_begin_; };

    auto integer_end() const -> const std::vector<std::size_t > &
    { return integer_end_; };

    auto string_length_begin() const -> const std::vector<std::size_t > &
    { return string_length_begin_; };

    auto string_length_end() const -> const std::vector<std::size_t > &
    { return string_length_end_; };

    auto string_length_begin() -> const std::vector<std::size_t > &
    { return string_length_begin_; };

    auto string_length_end() -> const std::vector<std::size_t > &
    { return string_length_end_; };

    auto list_begin() const -> const std::vector<std::size_t > &
    { return possible_list_begin_; };

    auto dict_begin() const -> const std::vector<std::size_t > &
    { return possible_dict_begin_; };

    auto end_positions() const -> const std::vector<std::size_t > &
    { return possible_end_; };

    auto possible_string_index() const -> std::span<const string_index_entry>
    { return string_index_; };

    auto possible_integer_index_() const -> std::span<const integer_index_entry>
    { return integer_index_; };

    void index(block_reader<64>& reader)
    {
        for (auto i = 0; i < reader.block_count(); ++i) {
            index_block(reader.read_next());
        }
//        index_possible_strings(reader);
//        index_possible_integers(reader);
    }

    void index_block(std::span<const std::byte, 64> input)
    {
        auto* test = reinterpret_cast<const char*>(input.data());
        __m256i lo     = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(input.data()));
        __m256i hi     = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(input.data()+32));
        __m256i c_lo   = bencode::detail::avx2::classify_characters(lo);
        __m256i c_hi   = bencode::detail::avx2::classify_characters(hi);

        std::uint32_t integer_begin_lo  = detail::avx2::get_integer_begin_mask(c_lo);
        std::uint32_t digits_lo         = detail::avx2::get_digits_mask(c_lo);
        std::uint32_t minus_lo          = detail::avx2::get_minus_mask(c_lo);
        std::uint32_t semicolon_lo      = detail::avx2::get_semicolon_mask(c_lo);
        std::uint32_t end_lo            = detail::avx2::get_end_symbol_mask(c_lo);
        std::uint32_t list_lo           = detail::avx2::get_list_start_mask(c_lo);
        std::uint32_t dict_lo           = detail::avx2::get_dict_start_mask(c_lo);
        std::uint32_t numeric_lo        = digits_lo | minus_lo;

        std::uint32_t integer_begin_hi  = detail::avx2::get_integer_begin_mask(c_hi);
        std::uint32_t digits_hi         = detail::avx2::get_digits_mask(c_hi);
        std::uint32_t minus_hi          = detail::avx2::get_minus_mask(c_hi);
        std::uint32_t semicolon_hi      = detail::avx2::get_semicolon_mask(c_hi);
        std::uint32_t end_hi            = detail::avx2::get_end_symbol_mask(c_hi);
        std::uint32_t list_hi           = detail::avx2::get_list_start_mask(c_hi);
        std::uint32_t dict_hi           = detail::avx2::get_dict_start_mask(c_hi);
        std::uint32_t numeric_hi        = digits_hi | minus_hi;

        // Load high and low bits of 64 bit integers in a register
        __m256i masks = _mm256_set_epi32(
                end_hi, end_lo, semicolon_hi, semicolon_lo,
                minus_hi, minus_lo, digits_hi, digits_lo);

        __m256i swapped    = detail::avx2::bit_swap64(masks);
        auto rdigits       = _mm256_extract_epi64(swapped, 0);
        auto rminus        = _mm256_extract_epi64(swapped, 1);
        auto rsemicolon    = _mm256_extract_epi64(swapped, 2);
        auto rend          = _mm256_extract_epi64(swapped, 3);
        auto rnumeric      = rdigits | rminus;

        // combine 32 bit masks to 64 bit
        auto integer_begin = detail::combine_bits(integer_begin_hi, integer_begin_lo);
        auto end           = detail::combine_bits(end_hi, end_lo);
        auto digits        = detail::combine_bits(digits_hi, digits_lo);
        auto semicolon     = detail::combine_bits(semicolon_hi, semicolon_lo);
        auto numeric       = detail::combine_bits(numeric_hi, numeric_lo);
        auto list          = detail::combine_bits(list_hi, list_lo);
        auto dict          = detail::combine_bits(dict_hi, dict_lo);

        detail::extract_indices_from_bitmask(list, position_, possible_list_begin_);
        detail::extract_indices_from_bitmask(dict, position_, possible_dict_begin_);
        detail::extract_indices_from_bitmask(end, position_,  possible_end_);

        get_integer_indices(integer_begin, end, rend, numeric, rnumeric);
        get_possible_string_lengths(semicolon, rsemicolon, digits, rdigits);
        position_ += 64;
    }

    void index_possible_strings(block_reader<64>& reader)
    {
        Expects(rng::size(string_length_begin_) == rng::size(string_length_end_));

        auto size = rng::size(string_length_begin_);
        auto begin_it = rng::begin(string_length_begin_);
        auto end_it   = rng::begin(string_length_end_);
        std::size_t idx = 0;
        string_index_.reserve(size);

        for ( ; idx < size/2 ; ++idx) {
            auto start1 = *begin_it++;
            auto start2 = *begin_it++;
            auto end1 = *end_it++;
            auto end2 = *end_it++;

            reader.seek(start1);
            auto span1 = reader.read();
            reader.seek(start2);
            auto span2 = reader.read();

            auto s1 = std::string_view(
                    reinterpret_cast<const char*>(rng::data(span1)) + start1, end1-start1);
            auto s2 = std::string_view(
                    reinterpret_cast<const char*>(rng::data(span2)) + start1, end2-start2);

            auto [i1, i2] = detail::avx2::convert_digits(s1, s2);
            string_index_.push_back({ .start=start1, .offset=end1-start1, .size=i1 });
            string_index_.push_back({ .start=start2, .offset=end2-start2, .size=i2 });
        }
        if ((size % 2) != 0) {
            auto start1 = *begin_it++;
            auto end1 = *end_it++;
            reader.seek(start1);
            auto span1 = reader.read();
            auto s1 = std::string_view(
                    reinterpret_cast<const char*>(rng::data(span1)) + start1, end1-start1);
            auto i1 = detail::avx2::convert_digits(s1);
            string_index_.push_back({ .start=start1, .offset=end1-start1, .size=i1 });
        }
    }

    void index_possible_integers(block_reader<64>& reader) {
        Expects(rng::size(integer_begin_) == rng::size(integer_end_));

        auto size = rng::size(integer_begin_);
        auto begin_it = rng::begin(integer_begin_);
        auto end_it   = rng::begin(integer_end_);
        std::size_t idx = 0;
        integer_index_.reserve(size);

        for ( ; idx < size ; ++idx) {
            auto start1 = *begin_it++;
            auto end1 = *end_it++;

            reader.seek(start1);
            auto span1 = reader.read();

            auto s1 = std::string_view(
                    reinterpret_cast<const char*>(rng::data(span1)) + start1, end1-start1);

            auto i1 = detail::avx2::convert_digits(s1);
            string_index_.push_back({ .start=start1, .offset=end1-start1, .size=i1 });
        }
    }

private:
    inline void get_integer_indices(std::uint64_t integer_begin,
            std::uint64_t end,
            std::uint64_t reverse_end,
            std::uint64_t numeric,
            std::uint64_t reverse_numeric)
    {
        // i_span is a range from i to the last digit in a run of numerics
        // shift integer_begin position one left so it aligns the start of a run of digits.
        // carry of the addition will flip digits to zero and set a one past the end of
        // the run of digits. We then x or with numeric to set to one the span of digits past and i.
        // next we shift one right so the range starts on the i and stop on the last digits instead.
        // Shift must be sign extended so runs that span or start at word boundaries
        // are correctly set.
        auto ti = (numeric + (integer_begin << 1)) ^ numeric;
        auto i_span  = static_cast<std::uint64_t>(static_cast<std::int64_t>(ti) >> 1);

        auto tre = (reverse_numeric + (reverse_end << 1)) ^ reverse_numeric;
        auto re_span = static_cast<std::uint64_t>(static_cast<std::int64_t>(tre) >> 1);
        auto e_span = detail::bit_swap(re_span);

        auto integer_range = i_span & e_span;
        // integer range does not include start and stop tokens so we must shift to get the indices.
        auto ibegin = (integer_range & (integer_begin << 1)) >> 1;
        auto iend   = (integer_range & (end >> 1)) << 1;

        // TODO: Becnhmark, maybe run this if highest and lowest bit are set.
        // Get the start and stop indices of integer token that span two data chunks.
        auto i_bounds_range   = detail::combine_bits(std::uint32_t(-1), previous_i_span_);
        // e span contains only e's preceded by digits if the first character if a new block
        // is an e it will not be set so we add the first character if it is an e.
        // We cannot add all end tokens since then successive e's would be erroneously added.
        auto e_bounds_range   = detail::combine_bits(detail::low_bits(e_span | (end & 1)),
                std::uint32_t(-1));
        // Bounds range includes i and e.
        auto bounds_range     = i_bounds_range & e_bounds_range;
        auto bounds_begin_offset = std::countl_one(detail::low_bits(bounds_range));
        auto bounds_end_offset   = std::countr_one(detail::high_bits(bounds_range));

        if (bounds_begin_offset + bounds_begin_offset > 2) [[unlikely]] {
            // trim the i and e from the positions
            integer_begin_.push_back(position_ - bounds_begin_offset);
            integer_end_.push_back(position_ + bounds_end_offset - 1);
        }

        // update leftover boundary span for next iteration
        previous_i_span_ = detail::high_bits(i_span);

        auto n1 = detail::extract_indices_from_bitmask(ibegin, position_, integer_begin_);
        auto n2 = detail::extract_indices_from_bitmask(iend, position_, integer_end_);
        Ensures(n1 == n2);
    }

    void get_possible_string_lengths(
            std::uint64_t semicolon,
            std::uint64_t reverse_semicolon,
            std::uint64_t digits,
            std::uint64_t reverse_digits)
    {
//        <----------------------------------------------------------------
//
//        23:2aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa:222:32:322:5
//        0001000000000000000000000000000000000000000000000000111011011101 span
//        0010000000000000000000000000000000000000000000000001000100100010 send
//        0001000000000000000000000000000000000000000000000000001001000101 sbegin
//
//        1010001001000000000000000000000000000000000000000000000000001000 rev_sbegin
//        0100010010001000000000000000000000000000000000000000000000000100 rev_send
//        1011101101110000000000000000000000000000000000000000000000001000 rev_span
//        5:223:23:222:aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa2:32
//
//         ---------------------------------------------------------------->

        auto ts = (reverse_digits + (reverse_semicolon << 1)) ^ reverse_digits;
        // span of digits preceded by semicolon.
        // we shift one to the right and have to set the zero that was shifted in if it is a digit.
//        auto reverse_span = ((ts >> 1) & ~reverse_semicolon) | (0x8000000000000000 & reverse_digits);
        auto reverse_span = static_cast<std::uint64_t>(static_cast<std::int64_t>(ts >> 1)
                            & ~reverse_semicolon);

        // reverse end is the position of the semicolon
        auto reverse_send = (reverse_span >> 1) & (reverse_semicolon);
        // find begin by carrying the semicolon through the run of digits, and shift one back
        auto reverse_sbegin = (reverse_span >> 1) + reverse_send;
        auto span = detail::bit_swap(reverse_span);
        auto send = detail::bit_swap(reverse_send);
        auto sbegin = detail::bit_swap(reverse_sbegin);

        // TODO: Benchmark, maybe run this if highest and lowest bit are set.
        // span ends on semicolon so find the lowest semicolon
        // then run it through the combined digits of this and previous run.
        auto lo_semicolon_index = std::countr_zero(semicolon);
        auto lo_semicolon_mask = 1 << lo_semicolon_index;
        auto before_lo_semicolon_mask = detail::bit_swap(std::uint32_t(0x80000000) >> lo_semicolon_index);
        auto digits_before_lo_semicolon = (digits & before_lo_semicolon_mask) | lo_semicolon_mask;
        auto bounds_range        = detail::combine_bits(detail::low_bits(digits_before_lo_semicolon),
                previous_digits_);
        auto bounds_begin_offset = std::countl_one(detail::low_bits(bounds_range));
        auto bounds_end_offset   = std::countr_one(detail::high_bits(bounds_range));

        // update the end of last written indices
        if ((bounds_end_offset > 0) && (bounds_begin_offset > 0)) [[unlikely]] {
            string_length_begin_.push_back(position_ - bounds_begin_offset);
            string_length_end_.push_back(position_ + bounds_end_offset - 1);
            detail::clear_lowest_bit(sbegin);
            detail::clear_lowest_bit(send);
        }

        previous_digits_ = detail::high_bits(digits);
        auto n1 = detail::extract_indices_from_bitmask(sbegin, position_, string_length_begin_);
        auto n2 = detail::extract_indices_from_bitmask(send, position_, string_length_end_);
        Ensures(n1 == n2);
    }

private:
    std::size_t position_ = 0;
    std::uint32_t previous_i_span_ = 0;
    std::uint32_t previous_digits_ = 0;

    std::vector<std::size_t> integer_begin_ {};
    std::vector<std::size_t> integer_end_ {};
    std::vector<std::size_t> string_length_begin_ {};
    std::vector<std::size_t> string_length_end_ {};
    std::vector<std::size_t> possible_list_begin_ {};
    std::vector<std::size_t> possible_dict_begin_ {};
    std::vector<std::size_t> possible_end_ {};

    std::vector<string_index_entry>  string_index_;
    std::vector<integer_index_entry> integer_index_;

    std::array<std::size_t, 32> begin_buffer_ {};
    std::array<std::size_t, 32> end_buffer_ {};
};


} // namespace bencode