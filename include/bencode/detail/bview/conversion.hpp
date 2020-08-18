
#pragma once


#include <tuple>
#include <nonstd/expected.hpp>

#include "bencode/detail/symbol.hpp"
#include "bencode/detail/utils.hpp"
#include "bencode/detail/exceptions.hpp"

#include "bencode/detail/concepts.hpp"
#include "bencode/detail/bview/concepts.hpp"
#include "bencode/detail/bview/bview.hpp"
#include "bencode/detail/bview/accessors.hpp"


namespace bencode::detail {

namespace rng = std::ranges;


// TODO: sync with more advanced bvalue conversions.


template <typename T>
constexpr auto convert_from_bview_to(const bview& ref) -> nonstd::expected<T, conversion_errc>
{
    // Check if there is a user-provided specialization found by ADL.
    if constexpr (conversion_from_bview_is_adl_overloaded<T>) {
        return bencode_convert_from_bview(customization_for<T>, std::forward<T>(ref));
    }
// Use bencode::serialisation_traits to split the overload set per bencode_type bvalue.
// This makes build errors easier to debug since we have to check less candidates.
    else if constexpr (serialization_traits<T>::type == bencode_type::integer) {
        return convert_from_bview_default_integer_impl<T>(
                customization_for<T>, ref);
    }
    else if constexpr (serialization_traits<T>::type == bencode_type::string) {
        return convert_from_bview_default_string_impl(
                customization_for<T>, ref, priority_tag<3>{});
    }
    else if constexpr (serialization_traits<T>::type == bencode_type::list) {
        return convert_from_bview_default_list_impl(
                customization_for<T>, ref, priority_tag<1>{});
    }
    else if constexpr (serialization_traits<T>::type == bencode_type::dict) {
        return convert_from_ref_default_dict_impl(
                customization_for<T>, ref, priority_tag<5>{});
    }
    else {
        static_assert(detail::always_false<T>::value,
                "no serializer for T found, check includes!");
    }
    return nonstd::make_unexpected(conversion_errc::undefined_conversion);
}


template <typename T>
    requires std::convertible_to<integer_bview, T>
constexpr nonstd::expected<T, conversion_errc>
convert_from_bview_default_integer_impl(customization_point_type<T>, const bview& ref)
{
    if (!is_integer(ref)) [[unlikely]] {
        return nonstd::make_unexpected(conversion_errc::not_integer_type);
    }
    return static_cast<T>(get_integer(ref));
}

// Conversion through std::string_view.

template <typename T>
    requires std::convertible_to<T, std::string_view>
constexpr nonstd::expected<T, conversion_errc>
convert_from_bview_default_string_impl(
        customization_point_type<T>,
        const bview& ref,
        priority_tag<2>) noexcept
{
    if (!is_string(ref)) [[unlikely]]
        return nonstd::make_unexpected(conversion_errc::not_integer_type);

    try { return T(std::string_view(get_string(ref))); }
    catch (...) { return nonstd::make_unexpected(conversion_errc::construction_error);
    }
}



// Conversion via iterators pair constructor

template <typename T>
    requires std::same_as<rng::range_value_t<T>, char> &&
             std::constructible_from<T, const char*, const char*>
constexpr nonstd::expected<T, conversion_errc>
convert_from_bview_default_string_impl(
        customization_point_type<T>,
        const bview& ref,
        priority_tag<1>) noexcept
{
    if (!is_string(ref)) [[unlikely]]
        return nonstd::make_unexpected(conversion_errc::not_string_type);

    const auto& bstring = get_string(ref);
    try { return T(rng::begin(bstring), rng::end(bstring)); }
    catch (...) { return nonstd::make_unexpected(conversion_errc::construction_error); }
}


// Conversion of  byte strings

template <typename T>
requires std::same_as<rng::range_value_t<T>, std::byte> &&
        std::constructible_from<T, const std::byte*, const std::byte*>
constexpr nonstd::expected<T, conversion_errc>
convert_from_bview_default_string_impl(
        customization_point_type<T>,
        const bview& b,
        priority_tag<0>)
{
    if (!is_string(b)) [[unlikely]]
        return nonstd::make_unexpected(conversion_errc::not_string_type);

    const auto& bstring = get_string(b);
    try { return T(reinterpret_cast<const std::byte*>(rng::data(bstring)),
                reinterpret_cast<const std::byte*>(rng::data(bstring)+rng::size(bstring))); }
    catch (...) { return nonstd::make_unexpected(conversion_errc::construction_error); }
}



// Conversion to list like types supporting front_inserter/back_inserter/inserter.

template <typename T>
requires std::constructible_from<T> && rng::sized_range<T> &&
        retrievable_from_bview<rng::range_value_t<T>> &&
        (supports_back_inserter<T> || supports_front_inserter<T> || supports_inserter<T>)
constexpr auto convert_from_bview_default_list_impl(
            customization_point_type<T>,
            const bview& ref,
            priority_tag<1>) -> nonstd::expected<T, conversion_errc>
{
    if (!is_list(ref)) [[unlikely]]
        return nonstd::make_unexpected(conversion_errc::not_list_type);

    const auto& blist = get_list(ref);

    try {
        T value{};
        if constexpr (supports_back_inserter<T>) {
            std::copy(rng::begin(blist), rng::end(blist), std::back_inserter(value));
        }
        else if constexpr (supports_front_inserter<T>) {
            std::copy(rng::rbegin(blist), rng::rend(blist), std::back_inserter(value));
        }
        else if constexpr (supports_inserter<T>) {
            std::copy(rng::begin(blist), rng::end(blist), std::inserter(value, rng::begin(value)));
        }
        return value;
    } catch (...) {
        return nonstd::make_unexpected(conversion_errc::construction_error);
    }
}

// Conversion to array

template <typename T>
requires std::constructible_from<T> &&
         has_tuple_like_structured_binding<T> &&
         convertible_from_bview_to_tuple_elements<T>
constexpr auto convert_from_bview_default_list_impl(
        customization_point_type<T>,
        const bview& ref,
        priority_tag<1>) -> nonstd::expected<T, conversion_errc>
{
    if (!is_list(ref)) [[unlikely]]
        return nonstd::make_unexpected(conversion_errc::not_list_type);

    const auto& blist = get_list(ref);

    if (rng::size(blist) != std::tuple_size_v<T>) [[unlikely]]
        return nonstd::make_unexpected(conversion_errc::size_mismatch);

    T out {};

    try {
        using IS = std::make_index_sequence<std::tuple_size_v<T>>;
        std::apply(
                [&]<std::size_t... IS>(std::index_sequence<IS...>&&) constexpr {
                    using std::get;
                    ( (get<IS>(out) = get_as<std::tuple_element_t<IS, T>>(blist[IS])) , ... );
                },
                std::forward_as_tuple(std::make_index_sequence<std::tuple_size_v<T>>{})
        );
        return out;
    } catch (...) {
        return nonstd::make_unexpected(conversion_errc::construction_error);
    }
}


template <key_value_associative_container T>
constexpr auto convert_from_bview_default_dict_impl (
        customization_point_type<T>,
        const bview& ref) ->  nonstd::expected<T, conversion_errc>
{
    if (is_dict(ref)) [[unlikely]]
        return nonstd::make_unexpected(conversion_errc::not_dict_type);

    const auto& bdict = get_dict(ref);

    try {
        T result {};
        std::transform(rng::begin(bdict), rng::end(bdict), std::inserter(result, rng::end(result)),
                       [](const auto& v) { return get_as<typename T::value_type>(v); });
        return result;
    }
    catch (...) {
        return nonstd::make_unexpected(conversion_errc::construction_error);
    }
}

} // namespace bencode::detail