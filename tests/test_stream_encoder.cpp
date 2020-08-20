//
// Created by fbdtemme on 09/07/19.
//
#include <string_view>
#include <catch2/catch.hpp>

#include <bencode/traits/all.hpp>
#include <bencode/detail/encoder.hpp>

namespace bc = bencode;


TEST_CASE("test stream encoder")
{
    using namespace std::string_view_literals;
    using namespace std::string_literals;

    std::ostringstream os {};
    auto emitter = bc::encoder(os);

    SECTION("integral") {
        emitter << true
                << std::int8_t{1}
                << std::int16_t{1}
                << std::int32_t{1}
                << std::int64_t{1}
                << std::uint8_t{1}
                << std::uint16_t{1}
                << std::uint32_t{1}
                << std::uint64_t{1};
        CHECK(os.str() == "i1ei1ei1ei1ei1ei1ei1ei1ei1e");
    }

    SECTION("string") {
        const char* const_char_ptr = "char ptr";

        emitter << "string literal"
                << "std::string_view"sv
                << "std::string"s
                << const_char_ptr;
        CHECK(os.str() == "14:string literal16:std::string_view11:std::string8:char ptr");
    }

    SECTION("list") {
        emitter << bc::begin_list
                << "string literal"
                << 1
                << "string"s
                << "string_view"sv
                << bc::end_list;
        CHECK(os.str() == "l14:string literali1e6:string11:string_viewe");
    }

    SECTION("dict") {
        emitter << bc::begin_dict
                << "key1"sv << 1
                << "key2"sv << "two"
                << "key3"sv
                    << bc::begin_list
                    << 1 << 2 << 3
                    << bc::end_list
                << bc::end_dict;
        std::cout << os.str() << std::endl;

        CHECK(os.str() == "d4:key1i1e4:key23:two4:key3li1ei2ei3eee");
    }

    SECTION("error - invalid dict key - integer") {
        emitter << bc::begin_dict;
        CHECK_THROWS_AS(emitter << 1, bc::encoding_error);
    }
    SECTION("error - invalid dict key - list") {
        emitter << bc::begin_dict;
        CHECK_THROWS_AS(emitter << bc::begin_list, bc::encoding_error);
        CHECK_THROWS_AS(emitter << bc::end_list, bc::encoding_error);
    }
    SECTION("error - invalid dict key - dict") {
        emitter << bc::begin_dict;
        CHECK_THROWS_AS(emitter << bc::begin_dict, bc::encoding_error);
    }

    SECTION("error - invalid dict end") {
        emitter << bc::begin_dict;
        emitter << "key1";
        emitter << bc::begin_list;
        emitter << 1;
        CHECK_THROWS_AS(emitter << bc::end_dict, bc::encoding_error);
    }

    SECTION("error - invalid list end") {
        emitter << bc::begin_list;
        emitter << bc::begin_dict << "key1";

        CHECK_THROWS_AS(emitter << bc::end_list, bc::encoding_error);
    }
}
