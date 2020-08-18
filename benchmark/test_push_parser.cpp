#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include <catch2/catch.hpp>
#include <chrono>

#include "bencode/bvalue.hpp"
#include "bencode/bview.hpp"
#include "bencode/detail/parser/common.hpp"
//#include "bencode/detail/parser/simd_operations.hpp"
//#include "bencode/detail/parser/simd/simd_parser.hpp"
#include "bencode/detail/parser/descriptor_parser.hpp"


#include <ranges>
#include <string_view>
#include <random>
#include <algorithm>
#include <filesystem>
#include <fstream>

using namespace std::string_view_literals;

namespace fs = std::filesystem;


TEST_CASE("benchmark parsers - Fedora-Workstation", "[parser]")
{
    std::ifstream ifs(RESOURCES_DIR"/Fedora-Workstation-Live-x86_64-30.torrent");
    std::string torrent(
            std::istreambuf_iterator<char>{ifs},
            std::istreambuf_iterator<char>{});

    BENCHMARK("decode")
    {
        auto d = bencode::decode_value(torrent);
        return d;
    };

    BENCHMARK("decode_view")
    {
        auto d = bencode::decode_view(torrent);
        return d;
    };
}


TEST_CASE("benchmark parsers - NASA mdim_color", "[parser]")
{
    std::ifstream ifs(RESOURCES_DIR"/NASA-Viking-Merged-Color-Mosaic.torrent");
    std::string torrent(
            std::istreambuf_iterator<char>{ifs},
            std::istreambuf_iterator<char>{});
    BENCHMARK("decode")
    {
        auto d = bencode::decode_value(torrent);
        return d;
    };

    BENCHMARK("decode_view")
    {
        auto d = bencode::decode_view(torrent);
        return d;
    };
}


TEST_CASE("benchmark parsers - COVID-19", "[parser]")
{
    std::ifstream ifs(RESOURCES_DIR"/COVID-19-image-dataset-collection.torrent");
    std::string torrent(
            std::istreambuf_iterator<char>{ifs},
            std::istreambuf_iterator<char>{});

    BENCHMARK("decode")
    {
        auto d = bencode::decode_value(torrent);
        return d;
    };

    BENCHMARK("decode_view")
    {
        auto d = bencode::decode_view(torrent);
        return d;
    };
}
