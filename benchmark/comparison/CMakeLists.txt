cmake_minimum_required(VERSION 3.15)

project("bencode-comparision-benchmark")

find_package(benchmark REQUIRED)
find_package(Boost REQUIRED)

include(../../external/libtorrent.cmake)

add_executable(benchmark-fbdtemme-bencode fbdtemme_bencode.cpp)
add_executable(benchmark-jimporter-bencode jimporter_bencode.cpp)
add_executable(benchmark-arvidn-libtorrent arvidn_libtorrent.cpp)

target_link_libraries(benchmark-jimporter-bencode PRIVATE Boost::headers)

foreach(target "benchmark-fbdtemme-bencode"
               "benchmark-jimporter-bencode"
               "benchmark-arvidn-libtorrent")
    target_link_libraries(${target} PRIVATE
            bencode::bencode
            benchmark::benchmark
            benchmark::benchmark_main
    )
    target_compile_definitions(${target} PRIVATE RESOURCES_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/../resources\")

endforeach(target)

target_link_libraries(benchmark-arvidn-libtorrent PRIVATE LibtorrentRasterbar::torrent-rasterbar)

