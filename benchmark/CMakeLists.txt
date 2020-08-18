cmake_minimum_required(VERSION 3.15)

include(CTest)
include(Catch)

add_executable(bencode-benchmark)
target_sources(bencode-benchmark
    PRIVATE
        main.cpp
        test_push_parser.cpp
)

target_compile_definitions(bencode-benchmark PRIVATE RESOURCES_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/resources\")
target_link_libraries(bencode-benchmark bencode::bencode Catch2::Catch2)
target_compile_options(bencode-benchmark PUBLIC)



catch_discover_tests(bencode-benchmark)
