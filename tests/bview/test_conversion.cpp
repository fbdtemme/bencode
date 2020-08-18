//
// Created by fbdtemme on 17/07/19.
//

#include <catch2/catch.hpp>

#include <utility>
#include "bencode/bencode.hpp"
#include "bencode/traits/all.hpp"
#include <bencode/bview.hpp>


namespace bc = bencode;

constexpr std::string_view data_integer = "i63e";
constexpr std::string_view data_string  = "4:spam";
constexpr std::string_view data_list    = "li2ei3ee";
constexpr std::string_view data_dict    = "d4:spami1ee";

constexpr std::array descriptors_integer = {
        bc::descriptor(bc::descriptor_type::integer, 0, 63L),
        bc::descriptor(bc::descriptor_type::stop, 4),
};

constexpr std::array descriptors_string  = {
        bc::descriptor(bc::descriptor_type::string, 0, 2U, 4U),
        bc::descriptor(bc::descriptor_type::stop, 6),
};

constexpr std::array descriptors_list = {
        bc::descriptor(bc::descriptor_type::list, 0, 2, 2),
        bc::descriptor(bc::descriptor_type::integer | bc::descriptor_type::list_value, 1, 2),
        bc::descriptor(bc::descriptor_type::integer | bc::descriptor_type::list_value, 4, 3),
        bc::descriptor(bc::descriptor_type::list | bc::descriptor_type::end, 7, 2, 2),
        bc::descriptor(bc::descriptor_type::stop, 8),
};

constexpr std::array descriptors_dict = {
        bc::descriptor(bc::descriptor_type::dict, 0, 3, 1),
        bc::descriptor(bc::descriptor_type::string  | bc::descriptor_type::dict_key, 1, 2, 4),
        bc::descriptor(bc::descriptor_type::integer | bc::descriptor_type::dict_value, 7, 1),
        bc::descriptor(bc::descriptor_type::dict | bc::descriptor_type::end, 10, 1, 3),
        bc::descriptor(bc::descriptor_type::stop, 11),
};

constexpr auto i_view_const = bc::bview(begin(descriptors_integer), data_integer.data());
constexpr auto s_view_const  = bc::bview(begin(descriptors_string), data_string.data());
constexpr auto l_view_const = bc::bview(begin(descriptors_list), data_list.data());
constexpr auto d_view_const = bc::bview(begin(descriptors_dict), data_dict.data());

static auto i_view = bc::bview(begin(descriptors_integer), data_integer.data());
static auto s_view  = bc::bview(begin(descriptors_string), data_string.data());
static auto l_view = bc::bview(begin(descriptors_list), data_list.data());
static auto d_view = bc::bview(begin(descriptors_dict), data_dict.data());



TEST_CASE("conversion from integer (bview)", "[bview][conversion][accessors]")
{
    auto n = 63;

    SECTION("bool") {
        auto i = get_as<bool>(i_view);
        CHECK(i == 1);
    }

    SECTION("int8_t") {
        auto as = get_as<int8_t>(i_view);
        CHECK(n == as);
    }

    SECTION("int16_t") {
        auto as = get_as<int16_t>(i_view);
        CHECK(n == as);
    }

    SECTION("int32_t") {
        auto as = get_as<int32_t>(i_view);
        CHECK(n == as);
    }

    SECTION("int64_t") {
        auto as = get_as<int64_t>(i_view);
        CHECK(n == as);
    }

    SECTION("uint8_t") {
        auto as = get_as<uint8_t>(i_view);
        CHECK(n == as);
    }

    SECTION("uint16_t") {
        auto as = get_as<uint16_t>(i_view);
        CHECK(n == as);
    }

    SECTION("uint32_t") {
        auto as = get_as<uint32_t>(i_view);
        CHECK(n == as);
    }

    SECTION("uint64_t") {
        auto as = get_as<uint64_t>(i_view);
        CHECK(n == as);
    }

    SECTION("integer literal") {
        auto as = get_as<int>(i_view);
        CHECK(as == 63);
    }

    SECTION("error - not integer type") {
        CHECK_THROWS_WITH(
                get_as<unsigned long long>(s_view), Catch::Contains("integer"));
        CHECK_THROWS_WITH(
                get_as<unsigned long long>(s_view_const), Catch::Contains("integer"));
    }
}


TEST_CASE("conversion from string (bview)", "[bview][conversion][accessors]")
{
    bencode::bvalue t_string = "spam";

    SECTION("std::vector<char>") {
        using T = std::vector<char>;
        auto v_string = get_as<T>(s_view);
        CHECK(v_string == t_string);
    }

    SECTION("std::vector<std::byte>") {
        using T = std::vector<std::byte>;
        auto v_string = get_as<T>(s_view);
        CHECK(v_string == t_string);
    }

    SECTION("error - not string type") {
        using T = std::string_view;
        CHECK_THROWS_WITH(get_as<T>(l_view), Catch::Contains("string"));
    }
}


TEST_CASE("conversion to tuple like list types", "[bview][conversion][accessors]")
{
    SECTION("std::tuple") {
        using T = std::pair<int, int>;

        auto val = get_as<T>(l_view);
        CHECK(std::get<0>(val) == 2);
        CHECK(std::get<1>(val) == 3);
    }

    SECTION("std::pair") {
        using T = std::pair<int, int>;
        auto val = get_as<T>(l_view);
        CHECK(std::get<0>(val) == 2);
        CHECK(std::get<1>(val) == 3);
    }

    SECTION("std::array") {
        using T = std::array<int, 2>;
        auto val = get_as<T>(l_view);
        CHECK(val[0] == 2);
        CHECK(val[1] == 3);
    }

    SECTION("error - not list type") {
        using T = std::array<int, 2>;
        CHECK_THROWS_WITH(get_as<T>(d_view), Catch::Contains("list"));
        CHECK_THROWS_WITH(get_as<T>(d_view_const), Catch::Contains("list"));
    }

    SECTION("error - size mismatch") {
        using T = std::array<int, 4>;
        CHECK_THROWS_WITH(get_as<T>(l_view), Catch::Contains("mismatch"));
        CHECK_THROWS_WITH(get_as<T>(l_view_const), Catch::Contains("mismatch"));
    }
}



TEST_CASE("conversions from list (bview)", "[bview][conversion][accessors]")
{
    std::string test1 = "long test string 1 to make sure there is no SSO and check if move is forwarded correctly";
    std::string test2 = "long test string 2 to make sure there is no SSO and check if move is forwarded correctly";

    SECTION("std::vector") {
        using T = std::vector<int>;
//
//        SECTION("list type") {
//            auto s = get_as<T>(l_view);
//            CHECK(s[0] == 2);
//            CHECK(s[1] == 3);
//        }

//        SECTION("error - not list type") {
//            CHECK_THROWS_WITH(get_as<T>(d_view), Catch::Contains("list"));
//        }
    }
//    SECTION("std::list") {
//        using T = std::list<std::string>;
//
//        SECTION("copy") {
//            auto v_string_copy = get_as<T>(t_list_of_string);
//            CHECK(v_string_copy.front() == test1);
//            CHECK(v_string_copy.back() == test2);
//            CHECK_FALSE(get_string(t_list_of_string[0]).empty());
//            CHECK_FALSE(get_string(t_list_of_string[1]).empty());
//
//        }
//        SECTION("move") {
//            auto v_string_move = get_as<T>(std::move(t_list_of_string));
//            CHECK(v_string_move.front() == test1);
//            CHECK(v_string_move.back() == test2);
//            CHECK(get_string(t_list_of_string[0]).empty());
//            CHECK(get_string(t_list_of_string[1]).empty());
//        }
//        SECTION("error - not list type") {
//            bencode::bvalue b{};
//            CHECK_THROWS_WITH(
//                    get_as<T>(b), Catch::Contains("list"));
//        }
//    }
//    SECTION("std::deque") {
//        using T = std::deque<std::string>;
//
//        SECTION("copy") {
//            auto v_string_copy = get_as<T>(t_list_of_string);
//            CHECK(v_string_copy[0]==test1);
//            CHECK(v_string_copy[1]==test2);
//            CHECK_FALSE(get_string(t_list_of_string[0]).empty());
//            CHECK_FALSE(get_string(t_list_of_string[1]).empty());
//
//        }
//        SECTION("move") {
//            auto v_string_move = get_as<T>(std::move(t_list_of_string));
//            CHECK(v_string_move[0]==test1);
//            CHECK(v_string_move[1]==test2);
//            CHECK(get_string(t_list_of_string[0]).empty());
//            CHECK(get_string(t_list_of_string[1]).empty());
//        }
//        SECTION("error - not list type") {
//            bencode::bvalue b{};
//            CHECK_THROWS_WITH(
//                    get_as<T>(b), Catch::Contains("list"));
//        }
//    }
//    SECTION("std::forward_list") {
//        using T = std::forward_list<std::string>;
//
//        SECTION("copy") {
//            auto v_string_copy = get_as<T>(t_list_of_string);
//            CHECK(*v_string_copy.begin() == test1);
//            CHECK(*std::next(v_string_copy.begin()) == test2);
//            CHECK_FALSE(get_string(t_list_of_string[0]).empty());
//            CHECK_FALSE(get_string(t_list_of_string[1]).empty());
//
//        }
//        SECTION("move") {
//            auto v_string_move = get_as<T>(std::move(t_list_of_string));
//            CHECK(*v_string_move.begin() == test1);
//            CHECK(*std::next(v_string_move.begin()) == test2);
//            CHECK(get_string(t_list_of_string[0]).empty());
//            CHECK(get_string(t_list_of_string[1]).empty());
//        }
//    }
//    SECTION("std::set") {
//        using T = std::set<std::string>;
//
//        SECTION("copy") {
//            auto v_string_copy = get_as<T>(t_list_of_string);
//            CHECK(*v_string_copy.find(test1) == test1);
//            CHECK(*v_string_copy.find(test2) == test2);
//            CHECK_FALSE(get_string(t_list_of_string[0]).empty());
//            CHECK_FALSE(get_string(t_list_of_string[1]).empty());
//
//        }
//        SECTION("move") {
//            auto v_string_move = get_as<T>(std::move(t_list_of_string));
//            CHECK(*v_string_move.find(test1) == test1);
//            CHECK(*v_string_move.find(test2) == test2);
//            CHECK(get_string(t_list_of_string[0]).empty());
//            CHECK(get_string(t_list_of_string[1]).empty());
//        }
//        SECTION("error - not list type") {
//            bencode::bvalue b{};
//            CHECK_THROWS_WITH(
//                    get_as<T>(b), Catch::Contains("list"));
//        }
//    }
//    SECTION("std::unordered_set") {
//        using T = std::unordered_set<std::string>;
//
//        SECTION("copy") {
//            auto v_string_copy = get_as<T>(t_list_of_string);
//            CHECK(*v_string_copy.find(test1) == test1);
//            CHECK(*v_string_copy.find(test2) == test2);
//            CHECK_FALSE(get_string(t_list_of_string[0]).empty());
//            CHECK_FALSE(get_string(t_list_of_string[1]).empty());
//
//        }
//        SECTION("move") {
//            auto v_string_move = get_as<T>(std::move(t_list_of_string));
//            CHECK(*v_string_move.find(test1) == test1);
//            CHECK(*v_string_move.find(test2) == test2);
//            CHECK(get_string(t_list_of_string[0]).empty());
//            CHECK(get_string(t_list_of_string[1]).empty());
//        }
//
//        SECTION("error - not list type") {
//            bencode::bvalue b{};
//            CHECK_THROWS_WITH(
//                    get_as<T>(b), Catch::Contains("list"));
//        }
//    }
//    SECTION("error - size_mismatch") {
//        using T = std::array<std::string, 0>;
//        CHECK_THROWS_WITH(get_as<T>(t_list_of_string), Catch::Contains("size mismatch"));
//    }
//    SECTION("error - not list type") {
//        using T = std::map<std::string, int>;
//        bvalue b = 4;
//        CHECK_THROWS_WITH(get_as<T>(b), Catch::Contains("not"));
//    }
//}
//        SECTION("copy") {
//            auto v_string_copy = get_as<T>(t_list_of_string);
//            CHECK(v_string_copy[0]==test1);
//            CHECK(v_string_copy[1]==test2);
//            CHECK_FALSE(get_string(t_list_of_string[0]).empty());
//            CHECK_FALSE(get_string(t_list_of_string[1]).empty());
//
//        }
//        SECTION("move") {
//            auto v_string_move = get_as<T>(std::move(t_list_of_string));
//            CHECK(v_string_move[0]==test1);
//            CHECK(v_string_move[1]==test2);
//            CHECK(get_string(t_list_of_string[0]).empty());
//            CHECK(get_string(t_list_of_string[1]).empty());
//        }
//        SECTION("error - not list type") {
//            bencode::bvalue b{};
//            CHECK_THROWS_WITH(
//                    get_as<T>(b), Catch::Contains("list"));
//        }
//    }
//    SECTION("std::list") {
//        using T = std::list<std::string>;
//
//        SECTION("copy") {
//            auto v_string_copy = get_as<T>(t_list_of_string);
//            CHECK(v_string_copy.front() == test1);
//            CHECK(v_string_copy.back() == test2);
//            CHECK_FALSE(get_string(t_list_of_string[0]).empty());
//            CHECK_FALSE(get_string(t_list_of_string[1]).empty());
//
//        }
//        SECTION("move") {
//            auto v_string_move = get_as<T>(std::move(t_list_of_string));
//            CHECK(v_string_move.front() == test1);
//            CHECK(v_string_move.back() == test2);
//            CHECK(get_string(t_list_of_string[0]).empty());
//            CHECK(get_string(t_list_of_string[1]).empty());
//        }
//        SECTION("error - not list type") {
//            bencode::bvalue b{};
//            CHECK_THROWS_WITH(
//                    get_as<T>(b), Catch::Contains("list"));
//        }
//    }
//    SECTION("std::deque") {
//        using T = std::deque<std::string>;
//
//        SECTION("copy") {
//            auto v_string_copy = get_as<T>(t_list_of_string);
//            CHECK(v_string_copy[0]==test1);
//            CHECK(v_string_copy[1]==test2);
//            CHECK_FALSE(get_string(t_list_of_string[0]).empty());
//            CHECK_FALSE(get_string(t_list_of_string[1]).empty());
//
//        }
//        SECTION("move") {
//            auto v_string_move = get_as<T>(std::move(t_list_of_string));
//            CHECK(v_string_move[0]==test1);
//            CHECK(v_string_move[1]==test2);
//            CHECK(get_string(t_list_of_string[0]).empty());
//            CHECK(get_string(t_list_of_string[1]).empty());
//        }
//        SECTION("error - not list type") {
//            bencode::bvalue b{};
//            CHECK_THROWS_WITH(
//                    get_as<T>(b), Catch::Contains("list"));
//        }
//    }
//    SECTION("std::forward_list") {
//        using T = std::forward_list<std::string>;
//
//        SECTION("copy") {
//            auto v_string_copy = get_as<T>(t_list_of_string);
//            CHECK(*v_string_copy.begin() == test1);
//            CHECK(*std::next(v_string_copy.begin()) == test2);
//            CHECK_FALSE(get_string(t_list_of_string[0]).empty());
//            CHECK_FALSE(get_string(t_list_of_string[1]).empty());
//
//        }
//        SECTION("move") {
//            auto v_string_move = get_as<T>(std::move(t_list_of_string));
//            CHECK(*v_string_move.begin() == test1);
//            CHECK(*std::next(v_string_move.begin()) == test2);
//            CHECK(get_string(t_list_of_string[0]).empty());
//            CHECK(get_string(t_list_of_string[1]).empty());
//        }
//    }
//    SECTION("std::set") {
//        using T = std::set<std::string>;
//
//        SECTION("copy") {
//            auto v_string_copy = get_as<T>(t_list_of_string);
//            CHECK(*v_string_copy.find(test1) == test1);
//            CHECK(*v_string_copy.find(test2) == test2);
//            CHECK_FALSE(get_string(t_list_of_string[0]).empty());
//            CHECK_FALSE(get_string(t_list_of_string[1]).empty());
//
//        }
//        SECTION("move") {
//            auto v_string_move = get_as<T>(std::move(t_list_of_string));
//            CHECK(*v_string_move.find(test1) == test1);
//            CHECK(*v_string_move.find(test2) == test2);
//            CHECK(get_string(t_list_of_string[0]).empty());
//            CHECK(get_string(t_list_of_string[1]).empty());
//        }
//        SECTION("error - not list type") {
//            bencode::bvalue b{};
//            CHECK_THROWS_WITH(
//                    get_as<T>(b), Catch::Contains("list"));
//        }
//    }
//    SECTION("std::unordered_set") {
//        using T = std::unordered_set<std::string>;
//
//        SECTION("copy") {
//            auto v_string_copy = get_as<T>(t_list_of_string);
//            CHECK(*v_string_copy.find(test1) == test1);
//            CHECK(*v_string_copy.find(test2) == test2);
//            CHECK_FALSE(get_string(t_list_of_string[0]).empty());
//            CHECK_FALSE(get_string(t_list_of_string[1]).empty());
//
//        }
//        SECTION("move") {
//            auto v_string_move = get_as<T>(std::move(t_list_of_string));
//            CHECK(*v_string_move.find(test1) == test1);
//            CHECK(*v_string_move.find(test2) == test2);
//            CHECK(get_string(t_list_of_string[0]).empty());
//            CHECK(get_string(t_list_of_string[1]).empty());
//        }
//
//        SECTION("error - not list type") {
//            bencode::bvalue b{};
//            CHECK_THROWS_WITH(
//                    get_as<T>(b), Catch::Contains("list"));
//        }
//    }
//    SECTION("error - size_mismatch") {
//        using T = std::array<std::string, 0>;
//        CHECK_THROWS_WITH(get_as<T>(t_list_of_string), Catch::Contains("size mismatch"));
//    }
//    SECTION("error - not list type") {
//        using T = std::map<std::string, int>;
//        bvalue b = 4;
//        CHECK_THROWS_WITH(get_as<T>(b), Catch::Contains("not"));
//    }
}
//
//TEST_CASE("conversions from dict", "[conversion][accessors]")
//{
//    std::string test1 = "long test string 1 to make sure there is no SSO and check if move is forwarded correctly";
//    std::string test2 = "long test string 2 to make sure there is no SSO and check if move is forwarded correctly";
//
//    auto t_dict = bencode::bvalue(btype::dict, {{"test1", test1}, {"test2", test2}});
//    auto t_multidict = bencode::bvalue(btype::dict, {
//            {"test1", bvalue(btype::list, {test1, test2})},
//            {"test2", test2}
//    });
//    auto t_dict_fail = bencode::bvalue(btype::dict, {
//            {"test1", test1},
//            {"test2", 3}
//    });
//    auto t_multidict_fail = bencode::bvalue(btype::dict, {
//            {"test1", bvalue(btype::list, {test1, 2})},
//            {"test2", test2}
//    });
//
//    SECTION("std::map") {
//        using T = std::map<std::string, std::string>;
//
//        SECTION("copy") {
//            auto v_dict_copy = get_as<T>(t_dict);
//            CHECK(v_dict_copy.contains("test1"));
//            CHECK(v_dict_copy.contains("test2"));
//            CHECK(get_string(t_dict["test1"])==test1);
//            CHECK(get_string(t_dict["test2"])==test2);
//        }
//
//        SECTION("move") {
//            auto v_dict_move = get_as<T>(std::move(t_dict));
//            CHECK(v_dict_move.contains("test1"));
//            CHECK(v_dict_move.contains("test2"));
//            CHECK(get_string(t_dict["test1"]).empty());
//            CHECK(get_string(t_dict["test2"]).empty());
//        }
//        SECTION("error - construction error") {
//            CHECK_THROWS_AS(get_as<T>(t_dict_fail), conversion_error);
//        }
//    }
//
//    SECTION("std::unordered_map") {
//        using T = std::unordered_map<std::string, std::string>;
//
//        SECTION("copy") {
//            auto v_dict_copy = get_as<T>(t_dict);
//            CHECK(v_dict_copy.contains("test1"));
//            CHECK(v_dict_copy.contains("test2"));
//            CHECK(get_string(t_dict["test1"])==test1);
//            CHECK(get_string(t_dict["test2"])==test2);
//        }
//
//        SECTION("move") {
//            auto v_dict_move = get_as<T>(std::move(t_dict));
//            CHECK(v_dict_move.contains("test1"));
//            CHECK(v_dict_move.contains("test2"));
//            CHECK(get_string(t_dict["test1"]).empty());
//            CHECK(get_string(t_dict["test2"]).empty());
//        }
//        SECTION("error - mapped type construction error") {
//            CHECK_THROWS_AS(get_as<T>(t_dict_fail), conversion_error);
//        }
//    }
//
//    SECTION("std::multi_map") {
//        using T = std::multimap<std::string, std::string>;
//
//        SECTION("copy") {
//            T v_dict_copy = get_as<T>(t_multidict);
//            CHECK(v_dict_copy.contains("test1"));
//            CHECK(v_dict_copy.contains("test2"));
//            auto& [k1, v1] = *v_dict_copy.find("test1");
//            CHECK((v1 == test1 || v1 == test2));
//            auto& [k2, v2] = *v_dict_copy.find("test2");
//            CHECK(v2 == test2);
//        }
//
//        SECTION("move") {
//            auto v_dict_move = get_as<T>(std::move(t_multidict));
//            CHECK(v_dict_move.contains("test1"));
//            CHECK(v_dict_move.contains("test2"));
//            auto& [k1, v1] = *v_dict_move.find("test1");
//            CHECK((v1 == test1 || v1 == test2));
//            auto& [k2, v2] = *v_dict_move.find("test2");
//            CHECK(v2 == test2);
//
//            CHECK(get_string(get_list(t_multidict["test1"])[0]).empty());
//            CHECK(get_string(t_multidict["test2"]).empty());
//        }
//
//        SECTION("error - mapped type construction error") {
//            CHECK_THROWS_AS(get_as<T>(t_multidict_fail), conversion_error);
//        }
//    }
//
//    SECTION("std::unordered_multimap") {
//        using T = std::unordered_multimap<std::string, std::string>;
//
//        SECTION("copy") {
//            T v_dict_copy = get_as<T>(t_multidict);
//            CHECK(v_dict_copy.contains("test1"));
//            CHECK(v_dict_copy.contains("test2"));
//            auto& [k1, v1] = *v_dict_copy.find("test1");
//            CHECK((v1 == test1 || v1 == test2));
//            auto& [k2, v2] = *v_dict_copy.find("test2");
//            CHECK((v2 == test1 || v2 == test2));
//        }
//
//        SECTION("move") {
//            auto v_dict_move = get_as<T>(std::move(t_multidict));
//            CHECK(v_dict_move.contains("test1"));
//            CHECK(v_dict_move.contains("test2"));
//            auto& [k1, v1] = *v_dict_move.find("test1");
//            CHECK((v1 == test1 || v1 == test2));
//            auto& [k2, v2] = *v_dict_move.find("test2");
//            CHECK(v2 == test2);
//
//            CHECK(get_string(get_list(t_multidict["test1"])[0]).empty());
//            CHECK(get_string(t_multidict["test2"]).empty());
//        }
//        SECTION("error - mapped type construction error") {
//            CHECK_THROWS_AS(get_as<T>(t_multidict_fail), conversion_error);
//        }
//    }
//    SECTION("error - not dict type") {
//        using T = std::map<std::string, int>;
//        bvalue b = 4;
//        CHECK_THROWS_WITH(get_as<T>(b), Catch::Contains("dict"));
//    }
//}
