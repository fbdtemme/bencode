//
// Created by fbdtemme on 8/17/20.
//
#include <catch2/catch.hpp>

#include <bencode/traits/all.hpp>
#include "bencode/bvalue.hpp"
#include <bencode/detail/events/consumer/debug_to.hpp>


#include <sstream>
namespace bc = bencode;

static const bc::bvalue b{
        {"a", 1},
        {"b", 1u},
        {"d", false},
        {"e", "string"},
        {"f", std::vector{1, 2, 3, 4}}
};

constexpr std::string_view b_events =
R"(begin dict (size=5)
begin list
string (size=1, value="a")
dict key
integer (1)
dict value
string (size=1, value="b")
dict key
integer (1)
dict value
string (size=1, value="d")
dict key
integer (0)
dict value
string (size=1, value="e")
dict key
string (size=6, value="string")
dict value
string (size=1, value="f")
dict key
begin list (size=4)
begin list
integer (1)
list item
integer (2)
list item
integer (3)
list item
integer (4)
list item
end list
dict value
end list
)";


TEST_CASE("producing events from bvalue", "[bvalue][events]")
{
    std::ostringstream os {};
    auto consumer = bc::events::debug_to(os);
    bc::connect(consumer, b);
    CHECK(os.str() == b_events);
}