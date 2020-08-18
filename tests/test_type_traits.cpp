//
// Created by fbdtemme on 26/03/19.
//

#include <catch2/catch.hpp>

#include <bencode/detail/concepts.hpp>
#include <bencode/bencode.hpp>

#include <map>
#include <unordered_map>
#include <vector>
#include <list>
#include <forward_list>
#include <valarray>
#include <array>


using namespace bencode;

TEST_CASE("test is_same_any")
{
   REQUIRE(detail::is_same_any_v<int, int, char>);
   REQUIRE(detail::is_same_any_v<int, std::tuple<int, char>>);



   std::int64_t test = std::size_t{1};
}


TEST_CASE("test is_equal_precision_unsigned_integer")
{
    int64_t i = uint32_t{1};
    CHECK(detail::is_equal_precision_unsigned_integer_v<std::int64_t, std::uint64_t>);
    CHECK(!detail::is_equal_precision_unsigned_integer_v<std::int32_t, std::uint64_t>);
    CHECK(detail::is_equal_precision_unsigned_integer_v<std::int64_t, std::uint32_t>);
}


TEST_CASE("test is_instantiation_of")
{
    CHECK(detail::is_instantiation_of<std::vector, std::vector<int>>::value);
    CHECK(detail::is_instantiation_of_v<std::vector, std::vector<int>>);
}

TEST_CASE("random test")
{
    using T = const char[1];

    bool a = !std::is_base_of_v<value, T>;
    bool b = !detail::is_same_any_v<T, bencode::bvalue::alternative_types>;
//    bool c = detail::has_trait_assign_v<bencode::bvalue, T>;
}

//
//TEST_CASE("test has_assign")
//{
////    REQUIRE(TraitAssignable<bencode::bvalue, bool>);
////    REQUIRE(TraitAssignable<bencode::bvalue, const char [1]>{});
//    CHECK(TraitAssignable<bencode::bvalue, const char*>);
//}


TEST_CASE("test is_iterable_container")
{
    REQUIRE(detail::is_iterable_container_v<std::list<int>>);
    REQUIRE(detail::is_iterable_container_v<std::vector<int>>);
    REQUIRE(detail::is_iterable_container_v<std::array<int, 20>>);
    REQUIRE(detail::is_iterable_container_v<std::valarray<int>>);
    REQUIRE(detail::is_iterable_container_v<std::forward_list<int>>);
}


TEST_CASE("test is_character")
{
    REQUIRE(detail::is_character_v<char>);
    REQUIRE(detail::is_character_v<wchar_t>);
    REQUIRE(detail::is_character_v<char16_t>);
    REQUIRE(detail::is_character_v<char32_t>);
    REQUIRE(!detail::is_character_v<std::vector<int>>);
    REQUIRE(!detail::is_character_v<bool>);
    REQUIRE(!detail::is_character_v<int>);
}


TEST_CASE("test is_dict_compatible")
{
    using string_type = bencode::bvalue::string_type;
    using t = std::map<std::string, bencode::bvalue>;
    using key = typename t::key_type;
    using mapped = typename t::mapped_type;
    using value_t = typename t::value_type;

    auto j = key{};
    auto k = mapped{};
    auto l = value_t{};

    REQUIRE(detail::is_dict_compatible_v<std::map<std::string, bencode::bvalue>>);
    REQUIRE(detail::is_dict_compatible_v<std::map<std::string, bencode::bvalue>>);
    REQUIRE(detail::is_dict_compatible_v<std::map<string_type, bencode::bvalue>>);
    REQUIRE(detail::is_dict_compatible_v<std::map<std::string, std::string>>);
    REQUIRE(detail::is_dict_compatible_v<std::map<const char*, bencode::bvalue>>);
    REQUIRE(detail::is_dict_compatible_v<std::multimap<string_type, bencode::bvalue>>);
    REQUIRE(detail::is_dict_compatible_v<std::unordered_map<string_type, bencode::bvalue>>);
    REQUIRE(detail::is_dict_compatible_v<std::unordered_multimap<string_type, bencode::bvalue>>);
}



TEST_CASE("test value_alternative_type")
{
    using T = bencode::value_alternative_t<bencode::bencode_type::string, bencode::bvalue>;
    REQUIRE(std::is_same_v<T, std::string>);
}
