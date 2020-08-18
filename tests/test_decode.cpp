//#include <catch2/catch.hpp>
//#include <type_traits>
//
//#include "../src/bencode.h"
//
//
//using NoView = std::false_type;
//using View = std::true_type;
//
//auto test_safe_integeral_parse(std::string_view arg) {
//    const char* temp;
//    return bencode::detail::safe_integral_parse<bencode::integer>(
//            std::begin(arg), std::end(arg), temp);
//}
//
//
//TEST_CASE("test safe_integral_parse", "[integer][string]") {
//    auto valid_value = "666";
//    auto zero = "0";
//    auto leading_zero = "000912e";
//    auto max_value = "9223372036854775807";
//    auto overflow = "9223372036854775808";
//    auto invalid_negative_value = "-12";
//    auto empty_string = "";
//
//    SECTION("valid bvalue") {
//        CHECK(test_safe_integeral_parse(valid_value) == 666);
//        CHECK(test_safe_integeral_parse(max_value) == 9223372036854775807);
//    }
//    SECTION("zero") {
//        CHECK(test_safe_integeral_parse(zero) == 0);
//    }
//    SECTION("leading zeros") {
//        CHECK_THROWS_WITH(
//                test_safe_integeral_parse(leading_zero),
//                "parse error - leading zero");
//    }
//    SECTION("overflow") {
//        CHECK_THROWS_WITH(
//                test_safe_integeral_parse(overflow),
//                "parse error - bvalue does not fit in receiving type");
//    }
//    SECTION("invalid negative integer") {
//        CHECK_THROWS_WITH(
//                test_safe_integeral_parse(invalid_negative_value),
//                "parse error - invalid integer: expected numeric character "
//                "but got '-'");
//    }
//    SECTION("empty string") {
//        CHECK_THROWS_WITH(
//                test_safe_integeral_parse(empty_string),
//                "parse error - invalid integer: empty input");
//    }
//}
//
//auto test_decode_int(std::string_view arg) {
//    auto temp = std::end(arg);
//    auto r = bencode::detail::decode_int<false>(
//            std::begin(arg),
//            std::end(arg),
//            temp
//    );
//    return r;
//}
//
//
//TEST_CASE("test decode_int", "[integer]") {
//    auto positive = "i666e";
//    auto negative = "i-666e";
//    auto zero = "i0e";
//    auto missing_end_token = "i666";
//    auto invalid_end_token= "i666k";
//
//    SECTION("positive values") {
//        CHECK(test_decode_int(positive) == 666);
//    }
//    SECTION("negative values") {
//        CHECK(test_decode_int(negative) == -666);
//    }
//    SECTION("zero") {
//        CHECK(test_decode_int(zero) == 0);
//    }
//    SECTION("unexpected end") {
//        CHECK_THROWS_WITH(
//                test_decode_int(missing_end_token),
//                "parse error - unexpected end of integer");
//    }
//    SECTION("invalid end token") {
//        CHECK_THROWS_WITH(
//                test_decode_int(invalid_end_token),
//                "parse error - missing integer end token: expected 'e' but got 'k'");
//    }
//}
//
//
//auto test_decode_string(std::string_view arg) {
//    auto temp = std::begin(arg);
//    auto r = bencode::detail::decode_str<false>(
//            std::begin(arg),
//            std::end(arg),
//            temp
//    );
//    return r;
//}
//
//auto test_decode_string(std::istream& s) {
//    std::istreambuf_iterator<char> begin(s);
//    std::istreambuf_iterator<char> end;
//    auto r = bencode::detail::decode_str<false>(begin, end, begin);
//    return r;
//}
//
//auto test_decode_string_view(std::string_view arg) {
//    auto temp = std::begin(arg);
//    auto r = bencode::detail::decode_str<true>(
//            std::begin(arg),
//            std::end(arg),
//            temp
//    );
//    return r;
//}
//
//
//TEST_CASE("test decode_string", "[string]") {
//    auto missing_string = "3";
//    auto invalid_length_seperator_token = "3f";
//    auto unexpected_eos = "3:fo";
//    std::stringstream unexpected_eos_stream;
//    unexpected_eos_stream << unexpected_eos;
//    std::string valid_string {"4:spam"};
//    std::stringstream valid_stream;
//    valid_stream << "4:spam";
//
//    SECTION("string") {
//        CHECK(test_decode_string(valid_string) == "spam");
//    }
//    SECTION("string - view") {
//        CHECK(test_decode_string_view(valid_string) == "spam");
//    }
//    SECTION("string - stream") {
//        CHECK(test_decode_string(valid_stream) == "spam");
//    }
//    SECTION("missing string after length specifier") {
//        CHECK_THROWS_WITH(
//                test_decode_string(missing_string),
//                "parse error - unexpected end of string");
//    }
//    SECTION("invalid string length seperator token") {
//        CHECK_THROWS_WITH(
//                test_decode_string(invalid_length_seperator_token),
//                "parse error - missing string length seperator token: "
//                "expected ':' but got 'f'");
//    }
//    SECTION("unexpected end of string - default") {
//        CHECK_THROWS_WITH(
//                test_decode_string(unexpected_eos),
//                "parse error - unexpected end of string");
//    }
//    SECTION("unexpected end of string - input iterator") {
//        CHECK_THROWS_WITH(
//                test_decode_string(unexpected_eos_stream),
//                "parse error - unexpected end of string");
//    }
//    SECTION("unexpected end of string - view") {
//        CHECK_THROWS_WITH(
//                test_decode_string_view(unexpected_eos),
//                "parse error - unexpected end of string");
//    }
//}
//
//template <bool AsView>
//auto test_decode_list_temp(std::string_view arg) {
//    auto temp = std::begin(arg);
//    auto r = bencode::detail::decode_list<AsView>(
//            std::begin(arg),
//            std::end(arg),
//            temp
//    );
//    return r;
//}
//
//const auto& test_decode_list = test_decode_list_temp<false>;
//const auto& test_decode_list_view = test_decode_list_temp<true>;
//
//
//TEST_CASE("test decode_list", "[list]") {
//    auto invalid_list_end = "l4:spami32e";
//    auto valid_list = "l4:spami32ee";
//
//    SECTION("valid list") {
//        auto test = test_decode_list(valid_list);
//        CHECK(std::is_same_v<decltype(test), bencode::list>);
//        CHECK(std::get<bencode::string>(test[0]) == "spam");
//        CHECK(std::get<bencode::integer>(test[1]) == 32);
//    }
//    SECTION("valid list - view") {
//        auto test = test_decode_list_view(valid_list);
//        CHECK(std::is_same_v<decltype(test), bencode::list_view>);
//        CHECK(std::get<bencode::string_view>(test[0]) == "spam");
//        CHECK(std::get<bencode::integer>(test[1]) == 32);
//    }
//
//    SECTION("invalid list") {
//        CHECK_THROWS_WITH(
//                test_decode_list(invalid_list_end),
//                "parse error - unexpected end of list");
//        CHECK_THROWS_WITH(
//                test_decode_list_view(invalid_list_end),
//                "parse error - unexpected end of list");
//    }
//}
//
//
//template <bool AsView>
//auto test_decode_dict_temp(std::string_view arg) {
//    auto temp = std::begin(arg);
//    auto r = bencode::detail::decode_dict<AsView>(
//            std::begin(arg),
//            std::end(arg),
//            temp
//    );
//    return r;
//}
//
//const auto& test_decode_dict = test_decode_dict_temp<false>;
//const auto& test_decode_dict_view = test_decode_dict_temp<true>;
//
//
//
//TEST_CASE("test decode_dict", "[dict]")
//{
//    auto valid_dict = "d4:spami32ee";
//    auto invalid_dict_key = "d:spami32ee";
//    auto duplicate_dict_key = "d4:spami32e4:spami53ee";
//    auto invalid_dict_end = "d4:spami32e";
//    auto non_sorted_dict =  "d4:spami32e3:fooi53ee";
//
//    SECTION("valid dict") {
//        auto test = test_decode_dict(valid_dict);
//        CHECK(std::is_same_v<decltype(test), bencode::dict>);
//        auto&[key, bvalue] = *test.begin();
//        CHECK(key=="spam");
//        CHECK(std::get<bencode::integer>(bvalue)==32);
//    }
//    SECTION("valid dict - view") {
//        auto test = test_decode_dict_view(valid_dict);
//        CHECK(std::is_same_v<decltype(test), bencode::dict_view>);
//        auto&[key, bvalue] = *test.begin();
//        CHECK(key=="spam");
//        CHECK(std::get<bencode::integer>(bvalue)==32);
//    }
//    SECTION("invalid dict key") {
//        CHECK_THROWS_WITH(test_decode_dict(invalid_dict_key),
//                "parse error - invalid dict key: expected string length");
//    }
//    SECTION("duplicate dict key") {
//        CHECK_THROWS_WITH(test_decode_dict(duplicate_dict_key),
//                "parse error - duplicated key in dict: 'spam'");
//    }
//    SECTION("invalid end of dict") {
//        CHECK_THROWS_WITH(test_decode_dict(invalid_dict_end),
//                "parse error - unexpected end of dict");
//    }
//    SECTION("unsorted keys") {
//        CHECK_THROWS_WITH(test_decode_dict(non_sorted_dict),
//                "parse error - unexpected end of dict");
//    }
//}
//
//TEST_CASE("test decode - stream", "[integer][string][list][dict]") {
//   SECTION("eof reached") {
//        std::stringstream stream_eof;
//        stream_eof << "d4:spami32ee";
//        auto res = std::get<bencode::dict>(
//                bencode::decode(stream_eof, bencode::eof_behavior::check));
//        CHECK(stream_eof.eof());
//
//    }
//    SECTION("use of eof_behavior::no_check") {
//        std::stringstream ss;
//        ss << "d4:spami32ee";
//        auto res1 = std::get<bencode::dict>(
//                bencode::decode(ss, bencode::eof_behavior::no_check));
//        CHECK(!ss.eof());
//        ss << "d4:foobi64ee";
//        auto res2 = std::get<bencode::dict>(
//                bencode::decode(ss, bencode::eof_behavior::no_check));
//        CHECK(!ss.eof());
//    }
//}
//
//
//TEST_CASE("decode successive objects from stream") {
//    std::stringstream data("i666e4:goat");
//
//    auto first = bencode::decode(data);
//    CHECK(std::get<bencode::integer>(first) == (666));
//    CHECK(!data.eof());
//
//    auto second = bencode::decode(data);
//    CHECK(std::get<bencode::string>(second) == "goat");
//    CHECK(data.eof());
//}
//
//
//
//TEST_CASE("test decode", "[integer][string][list][dict]") {
//    auto positive_int = "i666e";
//    std::stringstream stream_valid_int {positive_int};
//
//    SECTION("decode empty string") {
//        CHECK_THROWS_WITH(bencode::decode(""),
//                "parse error - unexpected end of data");
//    }
//    SECTION("invalid token") {
//        CHECK_THROWS_WITH(bencode::decode("k222e"),
//                "parse error - invalid type token: 'k'");
//    }
//
//    SECTION("test  values") {
//        CHECK(bencode::decode(positive_int) == bencode::data{666});
//    }
//}
//
//
//
//
