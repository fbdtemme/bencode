#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include <catch2/catch.hpp>
#include <chrono>

#include <ranges>
#include <string_view>
#include <fstream>

#include "jimporter_bencode.hpp"

using namespace std::string_view_literals;

namespace fs = std::filesystem;

inline void benchmark_helper(std::istream& ifs)
{
    std::string torrent(
            std::istreambuf_iterator<char>{ifs},
            std::istreambuf_iterator<char>{});

    BENCHMARK("decode_value") {
        auto d = bencode::decode(torrent);
        return d;
    };

    BENCHMARK("decode_view") {
         auto d = bencode::decode_view(torrent);
         return d;
    };
}

TEST_CASE("benchmark parsing - jimporter/bencode")
{
    SECTION("fedora workstation") {
        std::ifstream ifs(RESOURCES_DIR"/Fedora-Workstation-Live-x86_64-30.torrent");

        benchmark_helper(ifs);
    }

    SECTION("NASA mdim_color") {
        std::ifstream ifs(RESOURCES_DIR"/NASA-Viking-Merged-Color-Mosaic.torrent");
        benchmark_helper(ifs);
    }

    SECTION("COVID-19 image dataset") {
        std::ifstream ifs(RESOURCES_DIR"/COVID-19-image-dataset-collection.torrent");
        benchmark_helper(ifs);
    }
}