//#include <bencode/bencode_fwd.hpp>
//#include <bencode/detail/parser/input_adapter.hpp>
//#include <bencode/detail/parser/push_parser.hpp>
//#include <bencode/detail/events/consumers.hpp>
//#include "bencode/detail/parser/simd.hpp"
//
//#include <sstream>
//#include <string_view>
//
//#include <tl/expected.hpp>
//#include <bencode/detail/parser/error.hpp>
//
//#include <catch2/catch.hpp>
//
//TEST_CASE("test AVX2 ewfwef parsing")
//{
////    SECTION("invalid values in digit range") {
////        const char* str = "112ge323923e85";
////        auto r = bencode::detail::parse_integer_avx2(str);
////        CHECK_FALSE(r);
////    }
//
//    auto str = std::string_view("12345678012302345eeeeeeeeeeee");
//    auto r = bencode::avx2::parse_integer_full(str);
//    CHECK(r.has_value());
//    CHECK(*r == 12345678012302345);
//}
//
//
//using namespace std::string_view_literals;
//
//static constexpr auto str1 = "li33e4:spimi2el3:foo3:barei-333e"sv;
//static constexpr auto str_digits_mask =  0b01110000000100001001000000101100;
//
//
//TEST_CASE("decimal digits mask") {
//    const __m256i input = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(str1.data()));
//    auto r = bencode::avx2::decimal_digits_bitmask(input);
//    CHECK(r == str_digits_mask);
//};
//
//
//
//TEST_CASE("string length token mask")
//{
//    auto str = "li33e4:si99i2el3:foo3:barei-333e"sv;
//    const __m256i input = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(str.data()));
//    auto r = bencode::avx2::string_length_token_mask(input);
//}
