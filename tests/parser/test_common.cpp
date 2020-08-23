#include <catch2/catch.hpp>

#include <sstream>
#include <ranges>


#include <bencode/detail/parser/parsing_error.hpp>
#include <bencode/detail/parser/common.hpp>


using namespace std::string_view_literals;
using namespace bencode::detail;
namespace rng = std::ranges;

template <typename T>
auto valid_parse_integer_helper(std::string_view src, T result)
{
    const auto res = parse_integer<T>(rng::begin(src), rng::end(src));
    CHECK(res.has_value());
    CHECK(res.value() == result);
}

template <typename T>
auto invalid_parse_integer_helper(std::string_view src, bencode::parsing_errc ec)
{
    const auto res = parse_integer<T>(rng::begin(src), rng::end(src));
    CHECK_FALSE(res.has_value());
    CHECK(res.error() == ec);
}

TEST_CASE("parse integer", "[integer]")
{

    using namespace bencode;

    constexpr auto positive = "666"sv;
    constexpr auto negative = "-666"sv;
    constexpr auto zero = "0"sv;
    constexpr auto leading_zero = "000912e"sv;
    constexpr auto negative_zero = "-0"sv;
    constexpr auto empty = ""sv;
    constexpr auto minus_only = "-"sv;
    constexpr auto max_value = "9223372036854775807"sv;
    constexpr auto positive_overflow = "92233791812123120312116854775808"sv;
    constexpr auto negative_overflow = "-92233791812123120312116854775808"sv;

    SECTION("positive bvalue") {
        valid_parse_integer_helper(positive, 666);
        valid_parse_integer_helper(positive, 666UL);
    }
    SECTION("negative bvalue") {
        valid_parse_integer_helper(negative, -666);
    }
    SECTION("zero") {
        valid_parse_integer_helper(zero, 0);
        valid_parse_integer_helper(zero, 0UL);
    }
    SECTION("negative zero") {
        invalid_parse_integer_helper<std::int64_t>(negative_zero, parsing_errc::negative_zero);
    }
    SECTION("minus only") {
        invalid_parse_integer_helper<std::int64_t>(minus_only, parsing_errc::expected_digit);
    }
    SECTION("empty") {
        invalid_parse_integer_helper<std::int64_t>(empty, parsing_errc::expected_digit);
    }
    SECTION("leading zeros") {
        invalid_parse_integer_helper<std::int64_t>(leading_zero, parsing_errc::leading_zero);
        invalid_parse_integer_helper<std::size_t>(leading_zero, parsing_errc::leading_zero);
    }
    SECTION("positive overflow") {
        invalid_parse_integer_helper<std::int64_t>(positive_overflow, parsing_errc::integer_overflow);
        invalid_parse_integer_helper<std::size_t>(positive_overflow, parsing_errc::integer_overflow);
    }
    SECTION("negative overflow") {
        invalid_parse_integer_helper<std::int64_t>(negative_overflow, parsing_errc::integer_overflow);
    }
}

auto valid_bdecode_integer_helper(std::string_view src, std::int64_t result)
{
    const auto res = bdecode_integer<std::int64_t>(rng::begin(src), rng::end(src));
    CHECK(res.has_value());
    CHECK(res.value() == result);
}


auto invalid_bdecode_integer_helper(std::string_view src, bencode::parsing_errc ec)
{
    const auto res = bdecode_integer<std::int64_t>(rng::begin(src), rng::end(src));
    CHECK_FALSE(res.has_value());
    CHECK(res.error() == ec);
}


TEST_CASE("test bdecode_int", "[integer]") {
    using namespace bencode;

    auto positive = "i666e";
    auto negative = "i-666e";
    auto missing_end_token = "i666";
    auto invalid_end_token= "i666k";
    auto invalid_integer = "i-0e";
    auto empty = "";
    auto not_integer = "li1ee";

    SECTION("positive values") {
        valid_bdecode_integer_helper(positive, 666);
    }
    SECTION("negative values") {
        valid_bdecode_integer_helper(negative, -666);
    }
    SECTION("error - expected end token") {
        invalid_bdecode_integer_helper(missing_end_token, parsing_errc::expected_end);
        invalid_bdecode_integer_helper(invalid_end_token, parsing_errc::expected_end);
    }
    SECTION("error - negative zero") {
        invalid_bdecode_integer_helper(invalid_integer, parsing_errc::negative_zero);
    }
    SECTION("error - empty string") {
        invalid_bdecode_integer_helper(empty, parsing_errc::unexpected_eof);
    }
    SECTION("error - not an integer token") {
        invalid_bdecode_integer_helper(not_integer, parsing_errc::expected_integer_start_token);
    }
}



template <typename T>
auto valid_bdecode_string_helper(std::string_view src, std::string_view result)
{
    const auto res = bdecode_string<T>(rng::begin(src), rng::end(src));
    CHECK(res.has_value());
    CHECK(res.value() == result);
}


template <typename T>
auto invalid_bdecode_string_helper(std::string_view src, bencode::parsing_errc ec)
{
    const auto res = bdecode_string<T>(rng::begin(src), rng::end(src));
    CHECK_FALSE(res.has_value());
    CHECK(res.error() == ec);
}


TEMPLATE_TEST_CASE("test bdecode_string", "[string]", std::string, std::string_view)
{
    using namespace bencode;
    constexpr auto valid = "3:foo";
    constexpr auto missing_string = "3"sv;
    constexpr auto missing_seperator = "3f"sv;
    constexpr auto unexpected_eof = "3:fo"sv;
    constexpr auto empty = "";
    constexpr auto not_string = "i2e";
    constexpr auto negative_string_length = "-3:foo";

    std::stringstream valid_stream;
    valid_stream << "3:foo";

    SECTION("valid - view parsing") {
        valid_bdecode_string_helper<TestType>(valid, "foo");
    }
    SECTION("valid - non view parsing") {
        valid_bdecode_string_helper<TestType>(valid_stream.str(), "foo");
    }
    SECTION("missing colon") {
        invalid_bdecode_string_helper<TestType>(missing_string, parsing_errc::expected_colon);
    }
    SECTION("invalid seperator colon") {
        invalid_bdecode_string_helper<TestType>(missing_seperator, parsing_errc::expected_colon);
    }
    SECTION("invalid eof") {
        invalid_bdecode_string_helper<TestType>(unexpected_eof, parsing_errc::unexpected_eof);
    }
    SECTION("error - empty string") {
        invalid_bdecode_string_helper<TestType>(empty, parsing_errc::unexpected_eof);
    }
    SECTION("error - not a string token") {
        invalid_bdecode_string_helper<TestType>(not_string, parsing_errc::expected_digit);
    }
    SECTION("error - negative string length") {
        invalid_bdecode_string_helper<TestType>(negative_string_length, parsing_errc::negative_string_length);
    }
}

auto valid_bdecode_string_token_helper(std::string_view src)
{
    const auto res = bdecode_string_token(rng::begin(src), rng::end(src));
    CHECK(res.has_value());
    return *res;
}

auto invalid_bdecode_string_token_helper(std::string_view src, bencode::parsing_errc ec)
{
    const auto res = bdecode_string_token(rng::begin(src), rng::end(src));
    CHECK_FALSE(res.has_value());
    CHECK(res.error() == ec);
}


TEST_CASE("test bdecode_string_token", "[string]")
{
    using namespace bencode;
    constexpr auto valid = "3:foo"sv;
    constexpr auto missing_string = "3"sv;
    constexpr auto missing_seperator = "3f"sv;
    constexpr auto unexpected_eof = "3:fo"sv;
    constexpr auto empty = "";
    constexpr auto not_string = "i2e";
    constexpr auto negative_string_length = "-3:foo";

    SECTION("valid - view parsing") {
        auto r = valid_bdecode_string_token_helper(valid);
        CHECK(r.offset == 2);
        CHECK(r.size == 3);
    }
    SECTION("missing colon") {
        invalid_bdecode_string_token_helper(missing_string, parsing_errc::expected_colon);
    }
    SECTION("invalid seperator colon") {
        invalid_bdecode_string_token_helper(missing_seperator, parsing_errc::expected_colon);
    }
    SECTION("invalid eof") {
        invalid_bdecode_string_token_helper(unexpected_eof, parsing_errc::unexpected_eof);
    }
    SECTION("error - empty string") {
        invalid_bdecode_string_token_helper(empty, parsing_errc::unexpected_eof);
    }
    SECTION("error - not a string token") {
        invalid_bdecode_string_token_helper(not_string, parsing_errc::expected_digit);
    }
    SECTION("error - negative string length") {
        invalid_bdecode_string_token_helper(negative_string_length, parsing_errc::negative_string_length);
    }
}