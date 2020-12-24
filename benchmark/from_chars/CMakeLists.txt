cmake_minimum_required(VERSION 3.16)


find_package(benchmark REQUIRED)

add_executable(benchmark-from-chars benchmark.cpp)

target_compile_options(benchmark-from-chars PRIVATE)
foreach(target "benchmark-from-chars")
    target_link_libraries(${target}
            bencode::bencode
            benchmark::benchmark
            benchmark::benchmark_main
            )
    target_compile_definitions(${target} PRIVATE RESOURCES_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/../resources\")

endforeach(target)