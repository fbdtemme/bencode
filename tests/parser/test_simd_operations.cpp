////
//// Created by fbdtemme on 4/25/20.
////
//
//
//#include <string_view>
//#include <catch2/catch.hpp>
//
//
//#include "bencode/detail/parser/simd_operations.hpp"
//#include "bencode/detail/parser/simd/simd_parser.hpp"
//
//using namespace std::string_view_literals;
//namespace rng = std::ranges;
//
//struct classification_test_data
//{
//    std::string str;
//    std::string reverse_str;
//    std::array<std::uint8_t, 32> classification;
//    std::uint32_t digits;
//    std::uint32_t reverse_digits;
//    std::uint32_t reverse_semicolon;
//    std::uint32_t numerical;
//    std::uint32_t reverse_numerical;
//    std::uint32_t integer_start;
//    std::uint32_t reverse_integer_start;
//    std::uint32_t stop;
//    std::uint32_t reverse_stop;
//    std::uint32_t string_length_start;
//    std::uint32_t string_length_stop;
//};
//
//auto case1 = classification_test_data {
//    .str                   =  "li33e4:si99i2el3:foo3:barei-333e",
//    .reverse_str           =  "e333-ierab:3oof:3le2i99is:4e33il",
//    .classification        = { 16, 8, 1, 1, 64, 1, 4, 0, 8, 1, 1, 8, 1, 64, 16, 1,
//                                4, 0, 0, 0, 1, 4, 0, 0, 0, 64, 8, 2, 1, 1, 1, 64 },
//    .digits                = 0b01110000000100001001000000101100,
//    .reverse_digits        = 0b00110100000010010000100000001110,
//    .reverse_semicolon     = 0b00000010000000001000010000000000,
//    .numerical             = 0b01111000000100001001000000101100,
//    .reverse_numerical     = 0b00110100000010010000100000011110,
//    .integer_start         = 0b00000100000000000000100100000010,
//    .reverse_integer_start = 0b01000000100100000000000000100000,
//    .stop                  = 0b10000010000000000010000000010000,
//    .reverse_stop          = 0b00001000000001000000000001000001,
//    .string_length_start   = 0b00000000000100001000000000100000,
//    .string_length_stop    = 0b00000000001000010000000001000000,
//};
//
//
//auto case2 = classification_test_data {
//        .str                   =  "1223456ibskfiwmdpqowkvhdtawkd:20",
//        .reverse_str           =  "e333-ierab:3oof:3le2i99is:4e33il",
//        .classification        = { 16, 8, 1, 1, 64, 1, 4, 0, 8, 1, 1, 8, 1, 64, 16, 1,
//                                   4, 0, 0, 0, 1, 4, 0, 0, 0, 64, 8, 2, 1, 1, 1, 64 },
//        .digits                = 0b01110000000100001001000000101100,
//        .reverse_digits        = 0b00110100000010010000100000001110,
//        .reverse_semicolon     = 0b00000010000000001000010000000000,
//        .numerical             = 0b01111000000100001001000000101100,
//        .reverse_numerical     = 0b00110100000010010000100000011110,
//        .integer_start         = 0b00000100000000000000100100000010,
//        .reverse_integer_start = 0b01000000100100000000000000100000,
//        .stop                  = 0b10000010000000000010000000010000,
//        .reverse_stop          = 0b00001000000001000000000001000001,
//        .string_length_start   = 0b00000000000100001000000000100000,
//        .string_length_stop    = 0b00000000001000010000000001000000,
//};
//
//
//
//TEST_CASE("token classification")
//{
//    auto test_helper = [] (std::string_view str, rng::contiguous_range auto& result) {
//        const __m256i input = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(str.data()));
//        const __m256i expected = _mm256_loadu_si256(
//                reinterpret_cast<const __m256i*>(result.data()));
//
//        auto r = bencode::detail::avx2::classify_characters(input);
//
//        auto res = _mm256_cmpeq_epi8(r, expected);
//        return _mm256_movemask_epi8(res);
//    };
//
//    auto res_bit = test_helper(case1.str, case1.classification);
//    CHECK(res_bit == 0xFFFFFFFFFFFFFFFF);
//}
//
//TEST_CASE("test integer bit_reverse")
//{
//    std::uint64_t i = 0x1;
//    auto r = bencode::detail::bit_swap(i);
//    CHECK(r == 0x8000000000000000);
//
//    std::uint64_t i2 = 0x8000000000000001;
//    auto r2 = bencode::detail::bit_swap(i2);
//    CHECK(r2 == i2);
//}
//
//TEST_CASE("test vector bitflip")
//{
//    SECTION("32 bit") {
//        __m256i input = _mm256_set1_epi32(12);
//        auto result = bencode::detail::avx2::bit_swap32(input);
//        _mm256_cmpeq_epi8(result, _mm256_set1_epi32(805306368));
//    }
//
//    SECTION("64 bit") {
//        __m256i input = _mm256_set1_epi64x(12);
//        auto result = bencode::detail::avx2::bit_swap64(input);
//        _mm256_cmpeq_epi8(result, _mm256_set1_epi64x(3458764513820540928));
//    }
//}
//
//
////TEST_CASE("string length range")
////{
////    auto begin_expected = 0b00000100000000000000100000000010;
////    auto end_expected   = 0b10000000000000000010000000010000;
////
////    auto [begin, end] = bencode::detail::string_length_range_bitmask(
////            case1.reverse_semicolon,
////            case1.digits,
////            case1.reverse_digits);
////
////    CHECK(begin == case1.string_length_start);
////    CHECK(end == case1.string_length_stop);
////}
//
//
//TEST_CASE("extract indices")
//{
//    std::vector<std::size_t> indices {};
//
//    SECTION("less then 8") {
//        auto mask = 0b00000100000000000000100000000010;
//        std::vector<std::size_t> expected {1, 11, 26};
//
//        auto count = bencode::detail::extract_indices_from_bitmask(mask, 0UL, indices);
//        CHECK(count == 3);
//        CHECK(indices.size() == count);
//        CHECK(indices == expected);
//    }
//}
//
//
//static constexpr auto sintel_torrent = (
//"d8:announce40:udp://tracker.leechers-paradise.org:696913:announce-listll40:udp://tracker.leechers-paradise.org:6969el34:udp://tracker.coppersurfer.tk:6969el33:udp://tracker.opentrackr.org:1337el23:udp://explodie.org:6969el31:udp://tracker.empire-js.us:1337el26:wss://tracker.btorrent.xyzel32:wss://tracker.openwebtorrent.comel25:wss://tracker.fastcast.nzee"
//"7:comment34:WebTorrent <https://webtorrent.io>10:created by34:WebTorrent <https://webtorrent.io>13:creation datei1490916637e8:encoding5:UTF-8"
//"4:infod5:filesld6:lengthi1652e4:pathl13:Sintel.de.srteed6:lengthi1514e4:pathl13:Sintel.en.srteed6:lengthi1554e4:pathl13:Sintel.es.srteed6:lengthi1618e4:pathl13:Sintel.fr.srteed6:lengthi1546e4:pathl13:Sintel.it.srteed6:lengthi129241752e4:pathl10:Sintel.mp4eed6:lengthi1537e4:pathl13:Sintel.nl.srteed6:lengthi1536e4:pathl13:Sintel.pl.srteed6:lengthi1551e4:pathl13:Sintel.pt.srteed6:lengthi2016e4:pathl13:Sintel.ru.srteed6:lengthi46115e4:pathl10:poster.jpgeee4:name6:Sintel12:piece lengthi131072e6:pieces0:e"
//"8:url-listl31:https://webtorrent.io/torrents/ee"sv
//);
//
//static constexpr auto bounds_overlap =
//        "0000000000000000000000000000000000000000000000000000000000000000"
//        "infod5:filesld6:lengthi1652e4:pathl13:Sintel.de.srteed5:lengti12"      // <- integer cross word
//        "514e4:pathl13:Sintel.en.srteed6:lengthi1554e4:pathl6:Sinteleed60"      // <- string key cross word
//        ":aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaeee";
//
//static constexpr auto back_to_back_strings_keys =
//        "5:223:23:222:aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa2:32"
//        "31:bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
//
//TEST_CASE("chunk reader")
//{
//    auto reader = bencode::block_reader<64>(std::string_view(sintel_torrent));
//    auto chunk = reader.read();
//    CHECK(chunk.size() == 64);
//    auto s = std::string_view(reinterpret_cast<const char*>(chunk.data()), 64);
//    CHECK(std::string_view(sintel_torrent).starts_with(s));
//}
//
//
//static std::random_device rnd_device {};
//static std::mt19937 mersenne_engine {rnd_device()};
//
//auto d4_unsigned_dist = std::uniform_int_distribution<std::size_t>{0, 9999};
//auto d8_unsigned_dist = std::uniform_int_distribution<std::size_t>{10000, 99999999};
//auto d16_unsigned_dist = std::uniform_int_distribution<std::size_t>{100000000, 9999999999999999};
//auto d32_unsigned_dist = std::uniform_int_distribution<std::size_t>{
//        10000000000000000,
//        std::numeric_limits<std::size_t>::max()
//};
//
//template <typename Distribution>
//auto generate_test_data(Distribution d, int size)
//{
//    std::vector<std::pair<std::string, std::string_view>> data(size);
//    std::generate(data.begin(), data.end(),
//            [&]() {
//                auto r = std::to_string(d(mersenne_engine));
//                auto size = r.size();
//                r.resize(32, '\0');
//                return std::pair(std::move(r), std::string_view(r.data(), size));
//            });
//    return data;
//}
//
//TEST_CASE("integer conversion - 32 digits - avx2")
//{
//    SECTION("0 - 4 digits") {
//        auto data = generate_test_data(d4_unsigned_dist, 100);
//        for (auto[s, sv] : data) {
//            auto r = bencode::detail::avx2::convert_digits(sv);
//            CHECK(r==std::atoi(sv.data()));
//        }
//    }
//    SECTION("4 - 8 digits") {
//        auto data = generate_test_data(d8_unsigned_dist, 100);
//        for (auto[s, sv] : data) {
//            auto r = bencode::detail::avx2::convert_digits(sv);
//            CHECK(r==std::atoi(sv.data()));
//        }
//    }
//    SECTION("8 - 16 digits") {
//        auto data = generate_test_data(d16_unsigned_dist, 100);
//        for (auto[s, sv] : data) {
//            char* end;
//            auto r = bencode::detail::avx2::convert_digits(sv);
//            CHECK(r == std::strtoull(sv.data(), &end, 10));
//        }
//    }
//    SECTION("16 - max digits") {
//        auto data = generate_test_data(d32_unsigned_dist, 100);
//        for (auto[s, sv] : data) {
//            char* end;
//            auto r = bencode::detail::avx2::convert_digits(sv);
//            CHECK(r == std::strtoull(sv.data(), &end, 10));
//        }
//    }
//}
//
//
//TEST_CASE("avx2 - integer conversion")
//{
//    auto begin = std::array{69, 78, 92, 99, 118, 132, 139, 158, 172, 179};
//    auto end = std::array{70, 79, 93, 101, 119, 133, 141, 159, 173, 181};
//    auto data = std::string_view(bounds_overlap);
//    std::vector<std::uint32_t> results {};
//
//    SECTION("16 digit conversion") {
//        for (std::size_t idx = 0; idx<begin.size()-1; ++idx) {
//            auto s1 = data.substr(begin[idx], end[idx]-begin[idx]);
//            auto s2 = data.substr(begin[idx+1], end[idx+1]-begin[idx+1]);
//            auto[i1, i2] = bencode::detail::avx2::convert_digits(s1, s2);
//
//            CHECK(std::to_string(i1)==s1);
//            CHECK(std::to_string(i2)==s2);
//            results.push_back(i1);
//            results.push_back(i2);
//        }
//    }
//
//    SECTION("8 digit conversion") {
//        for (std::size_t idx = 0; idx < 2; ++idx) {
//            auto s1 = data.substr(begin[idx], end[idx]-begin[idx]);
//            auto s2 = data.substr(begin[idx+1], end[idx+1]-begin[idx+1]);
//            auto s3 = data.substr(begin[idx+2], end[idx+2]-begin[idx+2]);
//            auto s4 = data.substr(begin[idx+3], end[idx+3]-begin[idx+3]);
//
//            auto [i1, i2, i3, i4] = bencode::detail::avx2::convert_digits(s1, s2, s4, s4);
//
//            CHECK(std::to_string(i1) == s1);
//            CHECK(std::to_string(i2) == s2);
//            results.push_back(i1);
//            results.push_back(i2);
//        }
//    }
//
//}
//
//
//TEST_CASE("indexer - sintel")
//{
//    auto data = std::string_view(sintel_torrent);
//    auto reader = bencode::block_reader<64>(data);
//    auto indexer = bencode::token_indexer();
//
//    indexer.index(reader);
//
//    SECTION("possible list begin positions") {
//        for (auto l : indexer.list_begin()) {
//            CHECK(data[l] == 'l');
//        }
//    }
//    SECTION("possible dict begin positions") {
//        for (auto l : indexer.dict_begin()) {
//            CHECK(data[l] == 'd');
//        }
//    }
//    SECTION("possible end positions") {
//        for (auto l : indexer.end_positions()) {
//            CHECK(data[l] == 'e');
//        }
//    }
//
//    SECTION("integer positions") {
//        auto begin = indexer.integer_begin();
//        auto end   = indexer.integer_end();
//        CHECK(begin.size() == end.size());
//
//        for (std::size_t idx = 0; idx < begin.size(); ++idx) {
//            auto i = data[begin[idx]];
//            auto e = data[end[idx]];
//            CHECK(i == 'i');
//            CHECK(e == 'e');
//        }
//    }
//    SECTION("string length positions") {
//        auto begin = indexer.string_length_begin();
//        auto end   = indexer.string_length_end();
//        CHECK(begin.size() == end.size());
//
//        for (std::size_t idx = 0; idx < begin.size() ; ++idx) {
//            auto semicolon = data[end[idx]];
//            auto digit = data[begin[idx]];
//            CHECK(semicolon == ':');
//            CHECK(std::isdigit(digit));
//        }
//    }
//
////    SECTION("string index") {
////        indexer.index_possible_strings(reader);
////        auto r = indexer.possible_string_index();
////        for (auto i : r) {
////            auto size_str = std::to_string(i.size);
////            auto string_prefix = std::string_view(data.substr(i.start,i.offset));
////            CHECK( size_str == string_prefix );
////        }
////    }
//}
//
//
//TEST_CASE("indexer - corner cases")
//{
//    SECTION("integers cross word boundary") {
//        auto data = std::string_view(bounds_overlap);
//        auto reader = bencode::block_reader<64>(data);
//        auto indexer = bencode::token_indexer();
//
//        for (auto i = 0; i < reader.block_count(); ++i) {
//            indexer.index_block(reader.read_next());
//        }
//
//        SECTION("integer positions") {
//            auto begin = indexer.integer_begin();
//            auto end   = indexer.integer_end();
//            CHECK(begin.size() == end.size());
//            CHECK(begin.size() == 3);
//
//            for (std::size_t idx = 0; idx < begin.size() ; ++idx) {
//                auto i = data[begin[idx]];
//                auto e = data[end[idx]];
//                CHECK(i == 'i');
//                CHECK(e == 'e');
//            }
//        }
//
//        SECTION("string length positions") {
//            auto begin = indexer.string_length_begin();
//            auto end   = indexer.string_length_end();
//            CHECK(begin.size() == end.size());
//            CHECK(begin.size() == 11);
//
//            for (std::size_t idx = 0; idx < reader.block_count() ; ++idx) {
//                auto semicolon = data[end[idx]];
//                auto digit = data[begin[idx]];
//                CHECK(semicolon == ':');
//                CHECK(std::isdigit(digit));
//            }
//        }
//    }
//    SECTION("back_to_back_strings_keys") {
//        auto data = std::string_view(back_to_back_strings_keys);
//        auto reader = bencode::block_reader<64>(data);
//        auto indexer = bencode::token_indexer();
//
//        for (auto i = 0; i < reader.block_count(); ++i) {
//            indexer.index_block(reader.read_next());
//        }
//
//        SECTION("string length positions") {
//            auto begin = indexer.string_length_begin();
//            auto end   = indexer.string_length_end();
//            CHECK(begin.size() == end.size());
//            CHECK(begin.size() == 6);
//
//            for (std::size_t idx = 0; idx < reader.block_count() ; ++idx) {
//                auto semicolon = data[end[idx]];
//                auto digit = data[begin[idx]];
//                CHECK(semicolon == ':');
//                CHECK(std::isdigit(digit));
//            }
//        }
//    }
//}
//
//
////
////TEST_CASE("test parse string lengths")
////{
////    auto data = std::string_view(sintel_torrent);
////    auto reader = bencode::block_reader<64>(data);
////    auto indexer = bencode::token_indexer();
////
////    for (auto i = 0; i < reader.block_count(); ++i) {
////        indexer.index_block(reader.read_next());
////    }
////
////    auto begin = indexer.string_length_begin_positions();
////    auto end   = indexer.string_length_end_positions();
////    auto size = rng::size(indexer.string_length_begin_positions());
////
////    SECTION("avx2") {
////
////        auto r = bencode::detail::avx2::parse_string_lengths(
////                indexer.string_length_begin_positions(),
////                indexer.string_length_end_positions(),
////                reader);
////
////        CHECK(r.size()==size);
////
////        for (std::size_t idx; idx<size; ++idx) {
////            auto s = std::to_string(r[idx]);
////            auto r = std::string_view(data.substr(begin[idx], end[idx]-begin[idx]));
////            CHECK(s==r);
////        }
////    }
////
////    SECTION("serial") {
////        auto r = bencode::detail::avx2::parse_string_lengths(
////                indexer.string_length_begin_positions(),
////                indexer.string_length_end_positions(),
////                reader);
////
////        CHECK(r.size()==size);
////
////        for (std::size_t idx; idx<size; ++idx) {
////            auto s = std::to_string(r[idx]);
////            auto r = std::string_view(data.substr(begin[idx], end[idx]-begin[idx]));
////            CHECK(s==r);
////        }
////    }
////}
//
//
//TEST_CASE("simd_parser")
//{
//    auto data = std::string_view(sintel_torrent);
//    auto reader = bencode::block_reader<64>(data);
//    auto indexer = bencode::token_indexer();
//    auto parser = bencode::structural_index_parser{};
//
//
//    indexer.index(reader);
//
//    auto success = parser.parse(indexer, reader);
//}
