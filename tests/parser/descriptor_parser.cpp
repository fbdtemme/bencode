#include "bencode/bencode_fwd.hpp"
#include <bencode/bencode.hpp>
#include "bencode/detail/events/consumers.hpp"
#include "bencode/detail/parser/descriptor_parser.hpp"
#include <catch2/catch.hpp>

#include <iostream>
#include <fstream>
#include <string_view>
#include <sstream>
#include "../../include/bencode/detail/parser/descriptor_parser.hpp"

using namespace std::string_view_literals;
using namespace bencode;

constexpr auto example = (
        "d"
        "3:one"
        "i1e"
        "5:three"
        "l"
        "d"
        "3:bar" "i0e" "3:foo" "i0e"
        "e"
        "e"
        "3:two"
        "l"
        "i3e" "3:foo" "i4e"
        "e"
        "e"sv
);


TEST_CASE("index parser")
{
    using namespace bencode;

    auto p = descriptor_parser();
    auto r = p.parse(example);

    CHECK(r);
    auto index = std::move(r->descriptors());
    CHECK_FALSE(index.empty());

    CHECK(index.size() == 19);
    // check outer dict

    CHECK(index[0].is_dict_begin());
    CHECK(index[0].position() == 0);
    CHECK(index[0].offset() == 18);

    CHECK(index[18].is_dict_end());
    CHECK(index[18].position() == example.size()-1);
    CHECK(index[18].offset() == 18);

    // check first key bvalue pair ["one", 1]
    auto& k1 = index[1];
    auto& v1 = index[2];
    CHECK(k1.is_string());
    CHECK(k1.is_dict_key());
    CHECK(k1.position() == 1);
    CHECK(k1.offset() == 2);
    CHECK(v1.is_integer());
    CHECK(v1.is_dict_value());
    CHECK(v1.value() == 1);

    // check second key bvalue pair ["one", [{"bar": 0, "foo": 0}]]
    auto& k2 = index[3];
    auto& v2 = index[4];
    CHECK(k2.is_string());
    CHECK(k2.is_dict_key());
    CHECK(k2.position() == 9);
    CHECK(k2.offset() == 2);
    CHECK(v2.is_list());
    CHECK(v2.is_dict_value());
    CHECK(v2.position() == 16);
    CHECK(v2.size() == 1);
}


TEST_CASE("descriptor parser - COVID-19", "[descriptor_parser]")
{
    std::ifstream ifs(RESOURCES_DIR"/COVID-19-image-dataset-collection.torrent");
    std::string torrent(
            std::istreambuf_iterator<char>{ifs},
            std::istreambuf_iterator<char>{});

    auto parser = bencode::descriptor_parser();
    auto r = parser.parse(torrent);
    CHECK(r);
}
