
find_package(benchmark QUIET)
if (benchmark_FOUND)
    message(STATUS "Local installation of benchmark found.")
else()
    message(STATUS "Fetching dependency benchmark...")
    FetchContent_Declare(
            benchmark
            GIT_REPOSITORY https://github.com/google/benchmark.git
            GIT_TAG        master
    )
    set(BENCHMARK_ENABLE_TESTING OFF)
    set(BENCHMARK_ENABLE_INSTALL OFF)
    FetchContent_MakeAvailable(benchmark)
endif()

