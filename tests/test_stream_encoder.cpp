//
// Created by fbdtemme on 09/07/19.
//
#include <string_view>
#include <catch2/catch.hpp>

#include <bencode/traits/all.hpp>
#include <bencode/detail/encoding_ostream.hpp>


TEST_CASE("test stream encoder")
{
    using namespace std::string_view_literals;
    using namespace std::string_literals;

    std::ostringstream os {};
    auto emitter = bencode::encoding_ostream(os);

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
        emitter << bencode::begin_list
                << "string literal"
                << 1
                << "string"s
                << "string_view"sv
                << bencode::end_list;
        CHECK(os.str() == "l14:string literali1e6:string11:string_viewe");
    }

    SECTION("dict") {
        emitter << bencode::begin_dict
                << "key1"sv << 1
                << "key2"sv << "two"
                << "key3"sv
                    << bencode::begin_list
                    << 1 << 2 << 3
                    << bencode::end_list
                << bencode::end_dict;
        std::cout << os.str() << std::endl;

        CHECK(os.str() == "d4:key1i1e4:key23:two4:key3li1ei2ei3eee");
    }

    std::cout << os.str() << std::endl;
}
