#pragma once
#include <string_view>
#include <span>
#include <bit>
#include <ranges>
#include <cstdint>
//#include <experimental/simd>

// Load 32 chars in a 256-bit vector, we can do two of those in parallel.
// Classify chars using table lookup.
// Extract bitmaps of structural elements.
// Extract index positions from bitmaps
// Parse integers in parallel
// Build the descriptor list and verify input.

// TODO: benchmark if we should extract indices from a reversed classification vector
//       or bit reverse the normal bitmap

static_assert( -1 >> 1 == -1, "unsupported platform: signed right shift must be arithmetic");


namespace stdx = std::experimental;

namespace bencode {


namespace rng = std::ranges;


/// Read a buffer in blocks of size N.
/// Return the possible partial last block filled with zero bytes.
template <std::size_t N>
class block_reader
{
public:

    static constexpr std::size_t block_size = N;
    using value_type = std::span<const std::byte, block_size>;
    using char_value_type = std::string_view;

    block_reader(std::string_view buffer)
            : data_(reinterpret_cast<const std::byte*>(buffer.data()), buffer.size())
            , buffer_start_position_(buffer.size() - (buffer.size() % block_size))
    {
        std::fill_n(last_block_buffer_.data(), last_block_buffer_.size(), std::byte('\0'));

        std::size_t q = data_.size() / block_size;
        std::size_t r = data_.size() % block_size;
        block_count_ = q + ((r > 0) ? 1 : 0);
    }

    auto position() noexcept -> std::size_t
    { return position_; }

    auto read() const -> value_type
    {
        if (is_buffered()) [[unlikely]]
            return get_buffered_block(position_);

        return value_type(data_.data() + position_, N);
    }

    /// Interpret a block as characters
    auto read_chars() const -> std::string_view
    {
        if (is_buffered()) [[unlikely]] {
            auto t = get_buffered_block(position_);
            return std::string_view(reinterpret_cast<const char*>(t.data()), t.size());
        }
        return std::string_view(reinterpret_cast<const char*>(data_.data()) + position_, N);
    }

    auto is_buffered() const noexcept -> bool
    { return (position_ >= buffer_start_position_); }

    auto get_buffered_block(std::size_t pos) const noexcept -> value_type
    {
        // if data is 32 byte aligned we can return the data itself
        if (block_size == (data_.size() - buffer_start_position_)) [[unlikely]] {
            return value_type(data_.data() + buffer_start_position_, N);
        }
        // otherwise copy data to buffer, skip copying data to temp buffer if possible.
        if (std::to_integer<short>(*data_.begin()) != '\0') [[likely]] {
            std::copy(data_.begin() + buffer_start_position_, data_.end(),
                    last_block_buffer_.begin());
        }
        std::size_t offset = pos - buffer_start_position_;
        return value_type(last_block_buffer_.data() + offset, N);
    }

    constexpr auto block_count() noexcept -> std::size_t
    { return block_count_; }

    /// Return the size of the underlying buffer.
    constexpr auto size() noexcept -> std::size_t
    { return data_.size(); }

    constexpr void advance() noexcept
    { position_ += block_size; }

    // read and advance
    constexpr auto read_next() -> value_type
    {
        auto r = read();
        advance();
        return r;
    }

    /// Move position to chunk that contains the byte with index in data stream.
    constexpr auto seek(std::size_t pos)
    {
        position_ = pos;
    }

    /// Return a pointer to the original buffer interpreted as characters
    constexpr auto data() const noexcept -> const char*
    {
        return reinterpret_cast<const char*>(data_.data());
    }

private:
    std::span<const std::byte> data_;
    std::size_t position_ = 0;
    std::size_t block_count_;
    std::size_t buffer_start_position_;
    // buffer to store last 2 chunk of data padded with zeros
    mutable std::array<std::byte, N*2> last_block_buffer_ {};
};

} // namespace bencode