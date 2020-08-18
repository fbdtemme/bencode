#pragma once
//
#include <compare>
#include <concepts>

#include <bencode/detail/utils.hpp>
#include <bencode/detail/bvalue/concepts.hpp>
#include <bencode/detail/concepts.hpp>

//
namespace bencode::detail {

template <typename Policy, serializable T>
constexpr auto compare_equality_with_bvalue(const basic_bvalue<Policy>& bvalue, const T& value) -> bool
{
    // Check if there is a user-provided specialization found by ADL.
    if constexpr (equality_comparison_with_bvalue_is_adl_overloaded<T, Policy>) {
        return bencode_compare_equality_with_bvalue(customization_for<T>, bvalue, value);
    }
    else {
        // Use bencode::serialisation_traits to split the overload set per bencode_type bvalue.
        // This makes build errors easier to debug since we have to check less candidates.
        if constexpr (bencode::serialization_traits<T>::type == bencode_type::integer) {
            return compare_equality_with_bvalue_default_integer_impl(
                    customization_for<T>, bvalue, value, priority_tag<0>{});
        }
        else if constexpr (bencode::serialization_traits<T>::type == bencode_type::string) {
            return compare_equality_with_bvalue_default_string_impl(
                    customization_for<T>, bvalue,value, priority_tag<4>{});
        }
        else if constexpr (bencode::serialization_traits<T>::type == bencode_type::list) {
            return compare_equality_with_bvalue_default_list_impl(
                    customization_for<T>, bvalue,value, priority_tag<0>{});
        }
        else if constexpr (bencode::serialization_traits<T>::type == bencode_type::dict) {
            return compare_equality_with_bvalue_default_dict_impl(
                    customization_for<T>, bvalue,value, priority_tag<0>{});
        }
        else {
            static_assert(detail::always_false<T>::value, "no serializer for T found, check includes!");
        }
    }
    return false;
}

template <typename Policy, serializable T>
constexpr auto compare_three_way_with_bvalue(const basic_bvalue<Policy>& bvalue, const T& value) -> std::weak_ordering
{
    // Check if there is a user-provided specialization found by ADL.
    if constexpr (three_way_comparison_with_bvalue_is_adl_overloaded<T, Policy>) {
        return bencode_compare_three_way_with_bvalue(customization_for<T>, bvalue, value);
    }
    else {
        // Use bencode::serialisation_traits to split the overload set per bencode_type bvalue.
        // This makes build errors easier to debug since we have to check less candidates.
        if constexpr (bencode::serialization_traits<T>::type == bencode_type::integer) {
            return compare_three_way_with_bvalue_default_integer_impl(
                    customization_for<T>, bvalue, value, priority_tag<0>{});
        }
        else if constexpr (bencode::serialization_traits<T>::type == bencode_type::string) {
            return compare_three_way_with_bvalue_default_string_impl(
                    customization_for<T>, bvalue, value, priority_tag<2>{});
        }
        else if constexpr (bencode::serialization_traits<T>::type == bencode_type::list) {
            return compare_three_way_with_bvalue_default_list_impl(
                    customization_for<T>, bvalue, value, priority_tag<0>{});
        }
        else if constexpr (bencode::serialization_traits<T>::type == bencode_type::dict) {
            return compare_three_way_with_bvalue_default_dict_impl(
                    customization_for<T>, bvalue, value, priority_tag<0>{});
        }
        else {
            static_assert(detail::always_false<T>::value, "no serializer for T found, check includes!");
        }
    }
    Ensures(false);
}

//------------------------------------------------------------------------------------------------//
//  Equality comparison
//------------------------------------------------------------------------------------------------//


template <std::integral T, typename Policy>
    requires std::equality_comparable_with<policy_integer_t<Policy>, T>
constexpr auto compare_equality_with_bvalue_default_integer_impl(
            customization_point_type<T>,
            const basic_bvalue<Policy>& bvalue,
            T value,
            priority_tag<0>) -> bool
{
    if (!is_integer(bvalue)) return false;
    return (get_integer(bvalue) == value);
}

template <typename T, typename Policy>
    requires std::equality_comparable_with<policy_string_t<Policy>, T>
constexpr auto compare_equality_with_bvalue_default_string_impl(
        customization_point_type<T>,
        const basic_bvalue<Policy>& bvalue,
        const T& value,
        priority_tag<3>) -> bool
{
    if (!is_string(bvalue)) return false;
    return (get_string(bvalue) == value);
}

template <typename T, typename Policy>
    requires std::equality_comparable_with<rng::range_value_t<T>, char>
constexpr auto compare_equality_with_bvalue_default_string_impl(
        customization_point_type<T>,
        const basic_bvalue<Policy>& bvalue,
        const T& value,
        priority_tag<2>) -> bool
{
    if (!is_string(bvalue)) return false;
    const auto& s = get_string(bvalue);
    return std::equal(
            rng::begin(s), rng::end(s),
            rng::begin(value), rng::end(value));
}

/// Comparison for byte strings
template <typename T, typename Policy>
    requires std::same_as<rng::range_value_t<T>, std::byte>
constexpr auto compare_equality_with_bvalue_default_string_impl(
        customization_point_type<T>,
        const basic_bvalue<Policy>& bvalue,
        const T& value,
        priority_tag<1>) -> bool
{
    if (!is_string(bvalue)) return false;
    const auto& s = get_string(bvalue);
    return std::equal(
            rng::begin(s), rng::end(s),
            rng::begin(value), rng::end(value),
            [] (char lhs, std::byte rhs) {
                return short(lhs) == to_integer<short>(rhs);
            });
}

template <typename T, typename Policy>
    requires detail::has_string_member<T>
constexpr auto compare_equality_with_bvalue_default_string_impl(
        customization_point_type<T>,
        const basic_bvalue<Policy>& bvalue,
        const T& value,
        priority_tag<0>) -> bool
{
    if (!is_string(bvalue)) return false;
    const auto& s = get_string(bvalue);
    return s == value.string();
}


template <typename T, typename Policy>
    requires std::equality_comparable_with<policy_list_t<Policy>, T>
constexpr auto compare_equality_with_bvalue_default_list_impl(
        customization_point_type<T>,
        const basic_bvalue<Policy>& bvalue,
        const T& value,
        priority_tag<0>) -> bool
{
    if (!is_list(bvalue)) return false;
    return (get_list(bvalue) == value);
}


template <typename T, typename Policy>
    requires std::equality_comparable_with<policy_dict_t<Policy>, T>
constexpr auto compare_equality_with_bvalue_default_dict_impl(
        customization_point_type<T>,
        const basic_bvalue<Policy>& bvalue,
        const T& value,
        priority_tag<1>) -> bool
{
    if (!is_dict(bvalue)) return false;
    return (get_dict(bvalue) == value);
}


template <typename T, typename Policy>
constexpr auto compare_equality_with_bvalue_default_dict_impl(
        customization_point_type<T>,
        const basic_bvalue<Policy>& bvalue,
        const T& value,
        priority_tag<0>) -> bool
{
    if (!is_dict(bvalue)) return false;

    const auto& bdict = get_dict(bvalue);
    if (rng::size(bdict) != rng::size(value)) return false;

    return std::equal(rng::begin(bdict), rng::end(bdict),
                      rng::begin(value), rng::end(value),
                      [](const auto& p1, const auto& p2) {
                          return (p1.first == p2.first) && (p1.second == p2.second);
    }                );
}


//------------------------------------------------------------------------------------------------//
//  Three way comparison
//------------------------------------------------------------------------------------------------//


template <std::integral T, typename Policy>
    requires std::three_way_comparable_with<policy_integer_t<Policy>, T>
constexpr auto compare_three_way_with_bvalue_default_integer_impl(
        customization_point_type<T>,
        const basic_bvalue<Policy>& bvalue,
        T value,
        priority_tag<0>) -> std::weak_ordering
{
    if (!is_integer(bvalue)) return (bvalue.type() <=> bencode_type::integer);
    return (get_integer(bvalue) <=> value);
}

template <typename T, typename Policy>
    requires std::three_way_comparable_with<policy_string_t<Policy>, T>
constexpr auto compare_three_way_with_bvalue_default_string_impl(
        customization_point_type<T>,
        const basic_bvalue<Policy>& bvalue,
        const T& value,
        priority_tag<2>) -> std::weak_ordering
{
    if (!is_string(bvalue)) return (bvalue.type() <=> bencode_type::string);
    return (get_string(bvalue) <=> value);
}


template <typename T, typename Policy>
    requires std::three_way_comparable_with<rng::range_value_t<T>, char>
constexpr auto compare_three_way_with_bvalue_default_string_impl(
        customization_point_type<T>,
        const basic_bvalue<Policy>& b,
        const T& value,
        priority_tag<1>) -> std::weak_ordering
{
    if (!is_string(b)) return (b.type() <=> bencode_type::string);
    const auto& bstring = get_string(b);

    return std::lexicographical_compare_three_way(
            rng::begin(bstring), rng::end(bstring),
            rng::begin(value), rng::end(value));
}



/// Comparison for byte strings
template <typename T, typename Policy>
    requires std::same_as<rng::range_value_t<T>, std::byte>
constexpr auto compare_three_way_with_bvalue_default_string_impl(
        customization_point_type<T>,
        const basic_bvalue<Policy>& bvalue,
        const T& value,
        priority_tag<0>) -> std::weak_ordering
{
    if (!is_string(bvalue)) return (bvalue.type() <=> bencode_type::string);
    const auto& bstring = get_string(bvalue);
    return std::lexicographical_compare_three_way(
            rng::begin(bstring), rng::end(bstring),
            rng::begin(value), rng::end(value),
            [] (char lhs, std::byte rhs) {
                return short(lhs) <=> to_integer<short>(rhs);
            });
}

template <typename T, typename Policy>
    requires detail::has_string_member<T>
constexpr auto compare_three_way_with_bvalue_default_string_impl(
        customization_point_type<T>,
        const basic_bvalue<Policy>& bvalue,
        const T& value,
        priority_tag<0>) -> bool
{
    if (!is_string(bvalue)) return false;
    const auto& s = get_string(bvalue);
    return s <=> value.string();
}


template <typename T, typename Policy>
    requires std::three_way_comparable_with<policy_list_t<Policy>, T>
constexpr auto compare_three_way_with_bvalue_default_list_impl(
        customization_point_type<T>,
        const basic_bvalue<Policy>& bvalue,
        const T& value,
        priority_tag<1>) -> std::weak_ordering
{
    if (!is_list(bvalue)) return (bvalue.type() <=> bencode_type::list);
    return (get_list(bvalue) <=> value);
}


template <typename T, typename Policy>
constexpr auto compare_three_way_with_bvalue_default_list_impl(
        customization_point_type<T>,
        const basic_bvalue<Policy>& b,
        const T& value,
        priority_tag<0>) -> std::weak_ordering
{
    if (!is_string(b)) return (b.type() <=> bencode_type::string);
    const auto& blist = get_list(b);

    return std::lexicographical_compare_three_way(
            rng::begin(blist), rng::end(blist),
            rng::begin(value), rng::end(value));
}


template <typename T, typename Policy>
    requires std::three_way_comparable_with<policy_dict_t<Policy>, T>
constexpr auto compare_three_way_with_bvalue_default_dict_impl(
        customization_point_type<T>,
        const basic_bvalue<Policy>& bvalue,
        const T& value,
        priority_tag<1>) -> std::weak_ordering
{
    if (!is_dict(bvalue)) return (bvalue.type() <=> bencode_type::dict);
    return (get_dict(bvalue) <=> value);
}

template <typename T, typename Policy>
requires std::three_way_comparable_with<policy_dict_t<Policy>, T>
constexpr auto compare_three_way_with_bvalue_default_dict_impl(
        customization_point_type<T>,
        const basic_bvalue<Policy>& b,
        const T& value,
        priority_tag<0>) -> std::weak_ordering
{
    if (!is_dict(b)) return (b.type() <=> bencode_type::dict);
    const auto& bdict = get_dict(b);

    return std::lexicographical_compare_three_way(
            rng::begin(bdict), rng::end(bdict), rng::begin(value), rng::end(value),
            [](const auto& p1, const auto& p2) {
                return (p1.first==p2.first) && (p1.second==p2.second);
            });
}


} // namespace bencode::detail