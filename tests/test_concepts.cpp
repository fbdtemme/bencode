#include <string>
#include <string_view>
#include <vector>

#include <catch2/catch.hpp>
#include <bencode/traits/all.hpp>
#include <bencode/detail/concepts.hpp>
#include <bencode/bencode.hpp>
TEST_CASE("test serialisation_traits")
{
    using namespace bencode;
    CHECK(serializable_to<bool,          bencode_type::integer>);
    CHECK(serializable_to<std::int8_t,   bencode_type::integer>);
    CHECK(serializable_to<std::int16_t,  bencode_type::integer>);
    CHECK(serializable_to<std::int32_t,  bencode_type::integer>);
    CHECK(serializable_to<std::int64_t,  bencode_type::integer>);
    CHECK(serializable_to<std::uint8_t,  bencode_type::integer>);
    CHECK(serializable_to<std::uint16_t, bencode_type::integer>);
    CHECK(serializable_to<std::uint32_t, bencode_type::integer>);
    CHECK(serializable_to<std::uint64_t, bencode_type::integer>);

    CHECK(serializable_to<const char*,       bencode_type::string>);
    CHECK(serializable_to<const char[5],     bencode_type::string>);
    CHECK(serializable_to<std::string,       bencode_type::string>);
    CHECK(serializable_to<std::string_view,  bencode_type::string>);

    CHECK(serializable_to<std::array<int, 3>,      bencode_type::list>);
    CHECK(serializable_to<std::vector<int>,        bencode_type::list>);
    CHECK(serializable_to<std::forward_list<int>,  bencode_type::list>);
    CHECK(serializable_to<std::list<int>,          bencode_type::list>);
    CHECK(serializable_to<std::deque<int>,         bencode_type::list>);
    CHECK(serializable_to<std::set<int>,           bencode_type::list>);
    CHECK(serializable_to<std::multiset<int>,      bencode_type::list>);
    CHECK(serializable_to<std::unordered_set<int>,      bencode_type::list>);
    CHECK(serializable_to<std::unordered_multiset<int>, bencode_type::list>);

    CHECK(serializable_to<std::map<std::string, int>,                bencode_type::dict>);
    CHECK(serializable_to<std::multimap<std::string, int>,           bencode_type::dict>);
    CHECK(serializable_to<std::unordered_map<std::string, int>,      bencode_type::dict>);
    CHECK(serializable_to<std::unordered_multimap<std::string, int>, bencode_type::dict>);
}
//
//
//TEST_CASE("test trait_event_producer")
//{
//    using namespace bencode;
//
//    CHECK(trait_event_producer<std::string>);
//}
//
//
//TEST_CASE("test trait_equality_comparable")
//{
//    using namespace bencode;
//
//    bencode::bvalue b = "test";
//
//    SECTION("std::string") {
//        std::string s = "test";
//
////        CHECK(traits_old<std::string>::equal_to(b, s));
//        CHECK(trait_equality_comparable_with<std::string, bencode::bvalue>);
//    }
//
//    SECTION("std::string_view") {
//        using namespace std::string_view_literals;
//        auto s = "test"sv;
//
////        CHECK(traits_old<std::string_view>::equal_to(b, s));
//        CHECK(trait_equality_comparable_with<std::string_view, bencode::bvalue>);
//    }
//
//    SECTION("c string") {
//        using namespace std::string_view_literals;
//        auto s = "test";
//
//        CHECK(traits<const char*>::equal_to(b, s));
//        CHECK(trait_equality_comparable_with<const char*, bencode::bvalue>);
//    }
//}
//
//
//
//
//
//TEST_CASE("test trait_totally_ordered_with")
//{
//    using namespace bencode;
//
//    bencode::bvalue b = "test";
//
//    SECTION("std::string") {
//        std::string s = "test";
//
////        CHECK(traits_old<std::string>::compare_three_way(b, s) == std::partial_ordering::equivalent);
//        CHECK(trait_totally_ordered_with<std::string, bencode::bvalue>);
//    }
//
//    SECTION("std::string_view") {
//        using namespace std::string_view_literals;
//        auto s = "test"sv;
//
////        CHECK(traits_old<std::string_view>::compare_three_way(b, s)  == std::partial_ordering::equivalent);
//        CHECK(trait_totally_ordered_with<std::string_view, bencode::bvalue>);
//    }
//
//    SECTION("c string") {
//        using namespace std::string_view_literals;
//        auto s = "test";
//
//        CHECK(traits<const char*>::compare_three_way(b, s) == std::partial_ordering::equivalent);
//        CHECK(trait_totally_ordered_with<const char*, bencode::bvalue>);
//    }
//    SECTION("string literal") {
//        CHECK(traits<const char[4]>::compare_three_way(b, "test") == std::partial_ordering::equivalent);
//        CHECK(trait_totally_ordered_with<const char[4], bencode::bvalue>);
//    }
//}
//
