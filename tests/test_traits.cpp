////
//// Created by fbdtemme on 26/03/19.
////
//
//#ifndef LIBBENCODE_TEST_TRAITS_HPP
//#define LIBBENCODE_TEST_TRAITS_HPP
//
//
//#include <catch2/catch.hpp>
//
//#include "../bencode/traits_old.hpp"
//#include "../bencode/basic_bvalue.hpp"
//
//#include <cstdint>
//#include <vector>
//#include <list>
//#include <forward_list>
//#include <valarray>
//#include <array>
//#include <deque>
//#include <set>
//#include <unordered_set>
//
//
//using namespace bencode;
//
////TEST_CASE("test is_assignable")
////{
////    CHECK(detail::is_assignable_v<bencode::bvalue, std::array<char, 10>>);
////    CHECK(detail::is_assignable_v<bencode::bvalue, std::list<char>>);
////    CHECK(detail::is_assignable_v<bencode::bvalue, std::deque<char>>);
////    CHECK(detail::is_assignable_v<bencode::bvalue, std::forward_list<char>>);
////
////}
//
//TEST_CASE("test generic integral trait")
//{
//    CHECK(detail::is_assignable_v<bencode::bvalue, int>);
//    CHECK(detail::is_assignable_v<bencode::bvalue, uint16_t>);
//    CHECK(detail::is_assignable_v<bencode::bvalue, uint32_t>);
//    CHECK(detail::is_assignable_v<bencode::bvalue, uint64_t>);
//    CHECK(detail::is_assignable_v<bencode::bvalue, int8_t>);
//    CHECK(detail::is_assignable_v<bencode::bvalue, int16_t>);
//    CHECK(detail::is_assignable_v<bencode::bvalue, int32_t>);
//    CHECK(detail::is_assignable_v<bencode::bvalue, int64_t>);
//}

//
//TEST_CASE("test generic integral trait")
//{
//    using namespace bencode;
//    using arr = typename traits_old<std::array<bencode::bvalue, 4>>::array_type;
//
//    arr v{};
//    CHECK(detail::is_assignable_v<bencode::bvalue, std::array<bencode::bvalue, 4>>);
//}

//
//TEST_CASE("test generic list trait")
//{
//    CHECK(detail::is_assignable_v<bencode::bvalue, std::vector<int>>);
//    CHECK(detail::is_assignable_v<bencode::bvalue, std::array<int, 10>>);
//    CHECK(detail::is_assignable_v<bencode::bvalue, std::list<int>>);
//    CHECK(detail::is_assignable_v<bencode::bvalue, std::deque<int>>);
//    CHECK(detail::is_assignable_v<bencode::bvalue, std::forward_list<int>>);
//    CHECK(detail::is_assignable_v<bencode::bvalue, std::set<int>>);
//    CHECK(detail::is_assignable_v<bencode::bvalue, std::unordered_set<int>>);
//
//
//    // check constuction with non alternative type
//    using value_type = bool;
//
//    CHECK(detail::is_assignable_v<bencode::bvalue, std::vector<value_type>>);
//    CHECK(detail::is_assignable_v<bencode::bvalue, std::array<value_type, 10>>);
//    CHECK(detail::is_assignable_v<bencode::bvalue, std::list<value_type>>);
//    CHECK(detail::is_assignable_v<bencode::bvalue, std::deque<value_type>>);
//    CHECK(detail::is_assignable_v<bencode::bvalue, std::forward_list<value_type>>);
//    CHECK(detail::is_assignable_v<bencode::bvalue, std::set<value_type>>);
//    CHECK(detail::is_assignable_v<bencode::bvalue, std::unordered_set<value_type>>);
//
//
//}
//
//
//TEST_CASE("test generic qwdqw trait")
//{
////    CHECK(detail::is_iterable_container<int>{});
////    CHECK(std::is_convertible_v<const char*, std::string_view>);
//    CHECK(std::is_convertible_v<std::string_view, std::string_view>);
//}
//
//
//TEST_CASE("test generic string trait")
//{
//    CHECK(detail::is_assignable_v<bencode::bvalue, std::vector<char>>);
//    CHECK(detail::is_assignable_v<bencode::bvalue, std::array<char, 10>>);
//    CHECK(detail::is_assignable_v<bencode::bvalue, std::list<char>>);
//    CHECK(detail::is_assignable_v<bencode::bvalue, std::deque<char>>);
//    CHECK(detail::is_assignable_v<bencode::bvalue, std::forward_list<char>>);
//}
//
//
//
//TEST_CASE("test tuple trait")
//{
//    CHECK(detail::is_assignable_v<bencode::bvalue, std::tuple<int, std::string, bool>>);
//}
//
//
//
//#endif //LIBBENCODE_TEST_TRAITS_HPP
