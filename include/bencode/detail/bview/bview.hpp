#pragma once

#include <compare>
#include <cassert>

#include "bencode/detail/bview/concepts.hpp"
#include "bencode/detail/descriptor.hpp"


namespace bencode {

// forward declaration
class bview;


namespace detail {

constexpr const descriptor* get_storage(const bview& value) noexcept;

template <serializable T>
constexpr bool compare_equality_with_bview(const bview& bv, const T& value);

template <serializable T>
constexpr std::weak_ordering compare_three_way_with_bview(const bview& bv, const T& value);

constexpr bool compare_equality_bview_impl(const bview& lhs, const bview& rhs) noexcept;

constexpr std::weak_ordering compare_three_way_bview_impl(const bview& lhs, const bview& rhs) noexcept;

} // namespace detail


/// The class bview provides read-only access to a bencoded value.
/// The implementation holds only two members: a pointer pointing into contiguous sequence of descriptors
/// and pointer to the buffer containing the bencoded data.
/// Descriptors provide the structural information necessary to navigate the bencoded data.
/// The bview class and it's subclasses provide a convenient interface to interact with the bencoded data.
///
/// The bview class describes bencoded data of unknown type. The type can be queried by type().
/// bview subclasses describe bencoded data with a known type and can thus provide a richer interface.
class bview {
public:
    /// Default constructor. Constructs an empty bview.
    /// After construction, data() is equal to nullptr, and size() is equal to 0.
    constexpr bview() noexcept
        : desc_(nullptr), buffer_(nullptr) {}

    /// Constructs a bview from a descriptor and a character buffer.
    /// Behavior is undefined when data or buffer is nullptr
    constexpr bview(const descriptor* data, const char* buffer) noexcept
            : desc_(data)
            , buffer_(buffer)
    {
        Expects(data != nullptr);
        Expects(buffer != nullptr);
    }

    /// Copy constructor.
    constexpr bview(const bview&) noexcept = default;

    /// Copy assignment.
    constexpr bview& operator=(const bview&) noexcept = default;

    /// Returns the type of the descriptor this bview points to.
    /// @returns The bencode data type described by this bview.
    constexpr bencode::bencode_type type() const noexcept
    {
        if (desc_ == nullptr)          return bencode_type::uninitialized;
        if (desc_->is_integer())       return bencode_type::integer;
        if (desc_->is_string())        return bencode_type::string;
        if (desc_->is_list())          return bencode_type::list;
        if (desc_->is_dict())          return bencode_type::dict;
        BENCODE_UNREACHABLE;
    }
    
    /// Return the bencoded representation of the bvalue.
    /// @returns Bencoded representation of the described value.
    constexpr std::string_view bencoded_view() const noexcept
    {
        if (desc_->is_integer())
            return {buffer_, detail::base_ten_digits(desc_->value()) + 2 };
        else if (desc_->is_string())
            return {buffer_, desc_->offset() + desc_->size()};
        else {
            auto end_desc = std::next(desc_, desc_->offset());
            return {buffer_, end_desc->position() + 1 - desc_->position()};
        }
    }

    constexpr bool operator==(const bview& that) const noexcept
    {
        // defined in compare in avoid circular dependency with accessors
        return detail::compare_equality_bview_impl(*this, that);
    }

    constexpr std::weak_ordering operator<=>(const bview& that) const noexcept
    {
        // defined in compare in avoid circular dependency with accessors
        return detail::compare_three_way_bview_impl(*this, that);
    }

    /// Compares the current alternatives content with that.
    /// @param that the bvalue to compare to.
    /// @returns true of the lhs and rhs compare equal, false otherwise.
    template <typename T>
    /// \cond CONCEPTS
        requires (!std::same_as<T, bview> && !bview_alternative_type<T>)
    /// \endcond
    constexpr auto operator==(const T& that) const noexcept -> bool
    {
        return detail::compare_equality_with_bview(*this, that);
    }

    /// Compares the current alternatives content with that.
    /// @param that the bview whose content to compare
    /// @returns
    ///     std::partial_ordering::unordered if the current alternatives are of different types,
    ///     otherwise return the result of the comparison as a std::weak_ordering.
    template <typename T>
    /// \cond CONCEPTS
        requires (!std::same_as<T, bview> && !bview_alternative_type<T>)
    /// \endcond
    constexpr std::weak_ordering operator<=>(const T& that) const noexcept
    {
        return detail::compare_three_way_with_bview(*this, that);
    }

protected:
    friend constexpr const descriptor* detail::get_storage(const bview& value) noexcept;

    /// pointer to bencode data
    const char* buffer_;
    /// pointer into contiguous sequence of descriptors
    const descriptor* desc_;
};

} // namespace bencode

BENCODE_SERIALIZES_TO_RUNTIME_TYPE(bencode::bview);


