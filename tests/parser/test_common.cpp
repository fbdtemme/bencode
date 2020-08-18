#include <bencode/bencode_fwd.hpp>
#include <bencode/detail/parser/push_parser.hpp>
#include <bencode/detail/events/consumers.hpp>
#include <catch2/catch.hpp>
#include "bencode/detail/utils.hpp"

#include <sstream>
#include <ranges>

#include <nonstd/expected.hpp>
#include <bencode/detail/parser/error.hpp>
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
auto invalid_parse_integer_helper(std::string_view src, bencode::parser_errc ec)
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
        invalid_parse_integer_helper<std::int64_t>(negative_zero, parser_errc::negative_zero);
    }
    SECTION("minus only") {
        invalid_parse_integer_helper<std::int64_t>(minus_only, parser_errc::expected_digit);
    }
    SECTION("empty") {
        invalid_parse_integer_helper<std::int64_t>(empty, parser_errc::expected_digit);
    }
    SECTION("leading zeros") {
        invalid_parse_integer_helper<std::int64_t>(leading_zero, parser_errc::leading_zero);
        invalid_parse_integer_helper<std::size_t>(leading_zero, parser_errc::leading_zero);
    }
    SECTION("positive overflow") {
        invalid_parse_integer_helper<std::int64_t>(positive_overflow, parser_errc::integer_overflow);
        invalid_parse_integer_helper<std::size_t>(positive_overflow, parser_errc::integer_overflow);
    }
    SECTION("negative overflow") {
        invalid_parse_integer_helper<std::int64_t>(negative_overflow, parser_errc::integer_overflow);
    }
}

auto valid_bdecode_integer_helper(std::string_view src, std::int64_t result)
{
    const auto res = bdecode_integer<std::int64_t>(rng::begin(src), rng::end(src));
    CHECK(res.has_value());
    CHECK(res.value() == result);
}


auto invalid_bdecode_integer_helper(std::string_view src, bencode::parser_errc ec)
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

    SECTION("positive values") {
        valid_bdecode_integer_helper(positive, 666);
    }
    SECTION("negative values") {
        valid_bdecode_integer_helper(negative, -666);
    }
    SECTION("expected end token") {
        invalid_bdecode_integer_helper(missing_end_token, parser_errc::expected_end);
        invalid_bdecode_integer_helper(invalid_end_token, parser_errc::expected_end);
    }
    SECTION("error in integer parsing") {
        invalid_bdecode_integer_helper(invalid_integer, parser_errc::negative_zero);
    }
}



auto valid_bdecode_string_helper(std::string_view src, std::string_view result)
{
    const auto res = bdecode_string<std::string>(rng::begin(src), rng::end(src));
    CHECK(res.has_value());
    CHECK(res.value() == result);
}


auto invalid_bdecode_string_helper(std::string_view src, bencode::parser_errc ec)
{
    const auto res = bdecode_string<std::string>(rng::begin(src), rng::end(src));
    CHECK_FALSE(res.has_value());
    CHECK(res.error() == ec);
}


TEST_CASE("test bdecode_string", "[string]")
{
    using namespace bencode;
    constexpr auto valid = "3:foo";
    constexpr auto missing_string = "3"sv;
    constexpr auto missing_seperator = "3f"sv;
    constexpr auto unexpected_eof = "3:fo"sv;
    std::stringstream valid_stream;
    valid_stream << "3:foo";

    SECTION("valid - view parsing") {
        valid_bdecode_string_helper(valid, "foo");
    }
    SECTION("valid - non view parsing") {
        valid_bdecode_string_helper(valid_stream.str(), "foo");
    }
    SECTION("missing colon") {
        invalid_bdecode_string_helper(missing_string, parser_errc::expected_colon);
    }
    SECTION("invalid seperator colon") {
        invalid_bdecode_string_helper(missing_seperator, parser_errc::expected_colon);
    }
    SECTION("invalid eof") {
        invalid_bdecode_string_helper(unexpected_eof, parser_errc::unexpected_eof);
    }
}



auto valid_bdecode_string_view_helper(std::string_view src, std::string_view result)
{
    const auto res = bdecode_string_view<std::string_view>(rng::begin(src), rng::end(src));
    CHECK(res.has_value());
    CHECK(res.value() == result);
}


auto invalid_bdecode_string_view_helper(std::string_view src, bencode::parser_errc ec)
{
    const auto res = bdecode_string_view<std::string_view>(rng::begin(src), rng::end(src));
    CHECK_FALSE(res.has_value());
    CHECK(res.error() == ec);
}



TEST_CASE("test bdecode_string_view", "[string]")
{
    using namespace bencode;
    constexpr auto valid = "3:foo";
    constexpr auto missing_string = "3"sv;
    constexpr auto missing_seperator = "3f"sv;
    constexpr auto unexpected_eof = "3:fo"sv;

    SECTION("valid - view parsing") {
        valid_bdecode_string_view_helper(valid, "foo");
    }
    SECTION("missing colon") {
        invalid_bdecode_string_view_helper(missing_string, parser_errc::expected_colon);
    }
    SECTION("invalid seperator colon") {
        invalid_bdecode_string_view_helper(missing_seperator, parser_errc::expected_colon);
    }
    SECTION("invalid eof") {
        invalid_bdecode_string_view_helper(unexpected_eof, parser_errc::unexpected_eof);
    }
}