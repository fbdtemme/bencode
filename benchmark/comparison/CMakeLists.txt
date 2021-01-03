cmake_minimum_required(VERSION 3.15)

project("bencode-comparision-benchmark")

find_package(Boost REQUIRED COMPONENTS regex)

include(../../external/arvidn-libtorrent.cmake)
include(../../external/benchmark.cmake)
include(../../external/cpp-bencoding.cmake)
include(../../external/jimporter-bencode.cmake)
include(../../external/rakshasa-libtorrent.cmake)
include(../../external/kriben-bencode.cmake)

add_executable(benchmark-fbdtemme-bencode fbdtemme_bencode.cpp)
add_executable(benchmark-jimporter-bencode jimporter_bencode.cpp)
add_executable(benchmark-arvidn-libtorrent arvidn_libtorrent.cpp)
add_executable(benchmark-s3rvac-cpp-bencoding s3rvac_cpp_bencoding.cpp)
add_executable(benchmark-rakshasa-libtorrent rakshasa_libtorrent.cpp)
add_executable(benchmark-kriben-bencode kriben_bencode.cpp)

target_link_libraries(benchmark-jimporter-bencode PRIVATE Boost::headers)
target_link_libraries(benchmark-arvidn-libtorrent PRIVATE torrent-rasterbar)
target_link_libraries(benchmark-s3rvac-cpp-bencoding PRIVATE cpp-bencoding)
target_link_libraries(benchmark-jimporter-bencode PRIVATE jimporter-bencode)
target_link_libraries(benchmark-rakshasa-libtorrent PRIVATE rakshasa-libtorrent)
target_link_libraries(benchmark-kriben-bencode PRIVATE kriben-bencode Boost::headers Boost::regex)

foreach(target benchmark-fbdtemme-bencode
               benchmark-jimporter-bencode
               benchmark-arvidn-libtorrent
               benchmark-s3rvac-cpp-bencoding
               benchmark-rakshasa-libtorrent
               benchmark-kriben-bencode)

    target_link_libraries(${target} PRIVATE
            bencode::bencode
            benchmark::benchmark
            benchmark::benchmark_main
    )
    target_compile_definitions(${target} PRIVATE RESOURCES_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/../resources\")

endforeach(target)


add_custom_target(all-benchmarks)
add_dependencies(all-benchmarks
        benchmark-fbdtemme-bencode
        benchmark-jimporter-bencode
        benchmark-arvidn-libtorrent
        benchmark-s3rvac-cpp-bencoding
        benchmark-rakshasa-libtorrent
        benchmark-kriben-bencode)


