//#include <bencode/detail/parser/input_adapter.hpp>
#include <bencode/detail/parser/push_parser.hpp>
#include <bencode/detail/events/consumers.hpp>
#include <bencode/detail/bvalue/events.hpp>
#include <catch2/catch.hpp>
#include <sstream>

#include "data.hpp"

using namespace std::string_view_literals;
namespace bc = bencode;

TEST_CASE("test push parser to json")
{
    auto parser = bc::push_parser();
    std::string out{};
    auto consumer = bencode::events::format_json_to(std::back_inserter(out));

    SECTION("compare json output") {
        const auto [data, expected] = GENERATE_COPY(table<std::string_view, std::string_view>({
                {example,        example_json_result},
                {sintel_torrent, sintel_json_result}
        }));

        bool success = parser.parse(consumer, data);
        CHECK(out == expected);
    }

    SECTION("error - recursion limit") {
        auto r = parser.parse(consumer, recursion_limit);
        CHECK_FALSE(r);
        CHECK(parser.error().errc() == bc::parsing_errc::recursion_depth_exceeded);
    }

    SECTION("error - value limit") {
        auto parser2 = bencode::push_parser({.value_limit=10});
        auto r = parser2.parse(consumer, sintel_torrent);
        CHECK_FALSE(r);
        CHECK(parser2.error().errc() == bc::parsing_errc::value_limit_exceeded);
    }
//    auto parser2 = push_parser(data);
//    bencode::events::to_bvalue to_value_consumer{};
//    bool success2 = parser2.parse(to_value_consumer);
//    auto bvalue = to_value_consumer.bvalue();
}


