#include <array>
#include <string_view>
#include <tuple>

#include <catch2/catch.hpp>

#include <bencode/detail/events/consumer/encode_to.hpp>


template <std::size_t N>
constexpr std::pair<std::array<char, N>, std::size_t> to_iterator_test()
{
    using namespace bencode;
    using namespace std::string_view_literals;

    std::array<char, N> buffer {};

    auto consumer = bencode::events::encode_to(buffer.begin());
    consumer.begin_dict();
    consumer.string("test1"sv);
    consumer.dict_key();
    consumer.begin_list(4);
    consumer.integer(1);
    consumer.string("v2");
    consumer.integer(3);
    consumer.string("v4");
    consumer.end_list();
    consumer.string("test2"sv);
    consumer.dict_key();
    consumer.integer(2);
    consumer.end_dict();
    return {buffer, consumer.count()};
}


TEST_CASE("constexpr encoder to iterator")
{
    using namespace std::string_view_literals;
    constexpr auto p = to_iterator_test<100>();
    const auto view = std::string_view(p.first.data(), p.second);
    CHECK(view == "d5:test1li1e2:v2i3e2:v4e5:test2i2ee"sv);
}


//
////
//TEST_CASE("test integer encode", "[integer]") {
//    SECTION("positive integer") {
//        CHECK(bencode::encode(666)=="i666e");
//    }
//    SECTION("negative integer") {
//        CHECK(bencode::encode(-666)=="i-666e");
//    }
//    SECTION("positive zero") {
//        CHECK(bencode::encode(0) == "i0e");
//    }
//    SECTION("negative zero") {
//        CHECK(bencode::encode(-0)=="i0e");
//    }
//}

//
//TEST_CASE("test string encode", "[string]") {
//    SECTION("normal string") {
//        CHECK(bencode::encode("foo") == "3:foo");
//    }
//    SECTION("empty string") {
//        CHECK(bencode::encode("") == "0:");
//    }
//}
//
//
//TEST_CASE("test list encode", "[list]")
//{
//    auto expected = "l4:spami32ee";
//    auto expected_sorted = "li32e4:spame";
//
//    SECTION("empty list") {
//        CHECK(bencode::encode(bencode::list{}) == "le");
//    }
//
//    SECTION("std::vector list") {
//        std::vector<bencode::data> vector_list{"spam", 32};
//        CHECK(bencode::encode(vector_list) == expected);
//    }
//    SECTION("std::list list") {
//        std::list<bencode::data> list_list{"spam", 32};
//        CHECK(bencode::encode(list_list) == expected);
//    }
//    SECTION("std::deque list") {
//        std::deque<bencode::data> deque_list{"spam", 32};
//        CHECK(bencode::encode(deque_list) == expected);
//    }
//    SECTION("std::array list") {
//        std::array<bencode::data, 2> array_list{"spam", 32};
//        CHECK(bencode::encode(array_list) == expected);
//    }
//    SECTION("c-style array list") {
//        bencode::data c_style_array_list[2] = {"spam", 32};
//        CHECK(bencode::encode(c_style_array_list) == expected);
//    }
//    SECTION("std::set") {
//        std::set<bencode::data> set_list {"spam", 32};
//        CHECK(bencode::encode(set_list) == expected_sorted);
//    }
//    SECTION("std::unordered_set") {
//        std::unordered_set<bencode::data> set_list {"spam", 32};
//        CHECK(bencode::encode(set_list) == expected_sorted);
//    }
//
//    SECTION("vector of integers") {
//        std::vector<uint8_t> test(10);
//        std::iota(test.begin(), test.end(), 1);
//        CHECK(bencode::encode(test) == "li1ei2ei3ei4ei5ei6ei7ei8ei9ei10ee");
//    }
//    SECTION("static array of c-strings") {
//        const char* test[3] = {"foo", "bar", "baz"};
//        CHECK(bencode::encode(test) == "l3:foo3:bar3:baze");
//    }
//    SECTION("nested list types") {
//        bencode::list test {12, bencode::list{1, 2, 3}, "spam"};
//        CHECK(bencode::encode(test) == "li12eli1ei2ei3ee4:spame");
//    }
//    SECTION("vector of bytes") {
//        std::string bytes = "al;sadojqwpdqdmvp2941'}[s";
//        auto charBytes = (std::byte*)(bytes.data());
//
//        std::vector<std::byte> test(charBytes, charBytes+bytes.size());
//        CHECK(bencode::encode(test) == "25:al;sadojqwpdqdmvp2941'}[s");
//
//    }
//}
//
//struct dict_like_item {
//    std::string key;
//    int bvalue[3];
//};
//
//bool operator<(const dict_like_item& lhs, const dict_like_item& rhs) {
//    if (lhs.key == rhs.key)
//        return lhs.bvalue < rhs.bvalue;
//    return lhs.key < rhs.key;
//}
//
//struct dict_like: std::vector<dict_like_item> {
//    using key_type = std::string;
//    using mapped_type = int[3];
//    using base_type = std::vector<dict_like_item>;
//    using base_type::base_type;
//};
//
//
//TEST_CASE("test dict encode", "[dict]") {
//    auto test = bencode::dict{
//        {"one", 1},
//        {"two", "foo"},
//        {"three", 2}
//    };
//
//    SECTION("std::map") {
//       auto res = bencode::encode(test);
//       CHECK(res == "d3:onei1e5:threei2e3:two3:fooe");
//    }
//    SECTION("unsorted error") {
//        auto unordered_test = std::unordered_map<std::string, bencode::data>{
//                {"one", 1},
//                {"two", "foo"},
//                {"three", 2}
//        };
//       CHECK_THROWS_WITH(bencode::encode(unordered_test),
//               "encode error : dictionary keys are not sorted");
//    }
//    SECTION("dict like type - ordered") {
//        dict_like test {dict_like_item{"one", {1, 2, 3}},
//                        dict_like_item{"three", {4, 5, 6} }};
//
//        CHECK(bencode::encode(test) == "d3:oneli1ei2ei3ee5:threeli4ei5ei6eee");
//    }
//    SECTION("dict like type - unordered") {
//        dict_like test {dict_like_item{"three", {4, 5, 6} },
//                        dict_like_item{"one", {1, 2, 3}}};
//
//        CHECK_THROWS_WITH(bencode::encode(test),
//                "encode error : dictionary keys are not sorted");
//    }
//
//    SECTION("empty dict") {
//        CHECK(bencode::encode(bencode::dict{}) == "de");
//    }
//}
//
//
////
////suite<> test_encode("test encoder", [](auto &_) {
////
////    _.test("integer", []() {
////        expect(bencode::encode(666), equal_to("i666e"));
////        expect(bencode::encode(bencode::integer(666)), equal_to("i666e"));
////    });
////
////    _.test("string", []() {
////        expect(bencode::encode("foo"), equal_to("3:foo"));
////        expect(bencode::encode(std::string("foo")), equal_to("3:foo"));
////        expect(bencode::encode(bencode::string("foo")), equal_to("3:foo"));
////    });
////
////    _.test("list", []() {
////        expect(bencode::encode(bencode::list{}), equal_to("le"));
////        expect(bencode::encode(bencode::list{1, "foo", 2}),
////                equal_to("l" "i1e" "3:foo" "i2e" "e"));
////    });
////
////    _.test("dict", []() {
////        expect(bencode::encode(bencode::dict{}), equal_to("de"));
////        expect(bencode::encode(bencode::dict{
////                {"one", 1},
////                {"two", "foo"},
////                {"three", 2}
////        }), equal_to("d" "3:one" "i1e" "5:three" "i2e" "3:two" "3:foo" "e"));
////    });
////
////    _.test("nested", []() {
////        expect(bencode::encode(bencode::dict{
////                {"one", 1},
////                {"two", bencode::list{
////                        3, "foo", 4
////                }},
////                {"three", bencode::list{
////                        bencode::dict{
////                                {"foo", 0},
////                                {"bar", 0}
////                        }
////                }}
////        }), equal_to("d"
////                     "3:one" "i1e"
////                     "5:three" "l" "d" "3:bar" "i0e" "3:foo" "i0e" "e" "e"
////                     "3:two" "l" "i3e" "3:foo" "i4e" "e"
////                     "e"));
////    });
////
////    subsuite<>(_, "vector", [](auto &_) {
////        _.test("vector<int>", []() {
////            std::vector<int> v = {1, 2, 3};
////            expect(bencode::encode(v), equal_to("li1ei2ei3ee"));
////        });
////
////        _.test("vector<string>", []() {
////            std::vector<std::string> v = {"cat", "dog", "goat"};
////            expect(bencode::encode(v), equal_to("l3:cat3:dog4:goate"));
////        });
////
////        _.test("vector<vector<int>>", []() {
////            std::vector<std::vector<int>> v = {{1}, {1, 2}, {1, 2, 3}};
////            expect(bencode::encode(v), equal_to("lli1eeli1ei2eeli1ei2ei3eee"));
////        });
////    });
////
////    subsuite<>(_, "map", [](auto &_) {
////        _.test("map<string, int>", []() {
////            std::map<std::string, int> m = {{"a", 1}, {"b", 2}, {"c", 3}};
////            expect(bencode::encode(m), equal_to("d1:ai1e1:bi2e1:ci3ee"));
////        });
////
////        _.test("map<string, string>", []() {
////            std::map<std::string, std::string> m = {
////                    {"a", "cat"}, {"b", "dog"}, {"c", "goat"}
////            };
////            expect(bencode::encode(m), equal_to("d1:a3:cat1:b3:dog1:c4:goate"));
////        });
////
////        _.test("map<string, map<string, int>>", []() {
////            std::map<std::string, std::map<std::string, int>> m = {
////                    { "a", {{"a", 1}} },
////                    { "b", {{"a", 1}, {"b", 2}} },
////                    { "c", {{"a", 1}, {"b", 2}, {"c", 3}} }
////            };
////            expect(bencode::encode(m), equal_to(
////                    "d1:ad1:ai1ee1:bd1:ai1e1:bi2ee1:cd1:ai1e1:bi2e1:ci3eee"
////            ));
////        });
////    });
////
////});