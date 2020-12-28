cmake_minimum_required(VERSION 3.15)

project("bencode-comparision-benchmark")

find_package(benchmark REQUIRED)
find_package(Boost REQUIRED)

include(../../external/libtorrent.cmake)
add_subdirectory(external/s3rvac-cpp-bencoding/cpp-bencoding)

add_executable(benchmark-fbdtemme-bencode fbdtemme_bencode.cpp)
add_executable(benchmark-jimporter-bencode jimporter_bencode.cpp)
add_executable(benchmark-arvidn-libtorrent arvidn_libtorrent.cpp)
add_executable(benchmark-s3rvac-cpp-bencoding s3rvac_cpp_bencoding.cpp)

target_link_libraries(benchmark-jimporter-bencode PRIVATE Boost::headers)
target_link_libraries(benchmark-arvidn-libtorrent PRIVATE torrent-rasterbar)
target_link_libraries(benchmark-s3rvac-cpp-bencoding PRIVATE bencoding)

foreach(target "benchmark-fbdtemme-bencode"
               "benchmark-jimporter-bencode"
               "benchmark-arvidn-libtorrent"
               "benchmark-s3rvac-cpp-bencoding")
    target_link_libraries(${target} PRIVATE
            bencode::bencode
            benchmark::benchmark
            benchmark::benchmark_main
    )
    target_compile_definitions(${target} PRIVATE RESOURCES_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/../resources\")

endforeach(target)


