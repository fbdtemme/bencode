cmake_minimum_required(VERSION 3.15)

include(FetchContent)

find_package(Microsoft.GSL CONFIG QUIET)
if (Microsoft.GSL_FOUND)
    message(STATUS "Local installation of Microsoft.GSL found.")
else()
    message(STATUS "Fetching dependency Microsoft.GSL...")
    FetchContent_Declare(
            Microsoft.GSL
            GIT_REPOSITORY https://github.com/microsoft/GSL.git
            GIT_TAG        master
    )
    FetchContent_MakeAvailable(Microsoft.GSL)
endif()

find_package(fmt QUIET)
if (fmt_FOUND)
    message(STATUS "Local installation of fmt found.")
else()
    message(STATUS "Fetching dependency fmt...")
    FetchContent_Declare(
            fmt
            GIT_REPOSITORY https://github.com/fmtlib/fmt.git
            GIT_TAG        master
    )
    set(FMT_INSTALL ON)
    set(BUILD_SHARED_LIBS ON)
    FetchContent_MakeAvailable(fmt)
endif()


find_package(expected-lite QUIET)
if (expected-lite_FOUND)
    message(STATUS "Local installation of expected-lite found.")
else()
    message(STATUS "Fetching dependency expected-lite...")
    FetchContent_Declare(
            expected-lite
            GIT_REPOSITORY https://github.com/martinmoene/expected-lite.git
            GIT_TAG        master
    )
    FetchContent_MakeAvailable(expected-lite)
endif()


find_package(Catch2 QUIET)
if (Catch2_FOUND)
    message(STATUS "Local installation of Catch2 found.")
else()
    message(STATUS "Fetching dependency Catch2...")
    FetchContent_Declare(
            Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG        master
    )
    FetchContent_MakeAvailable(Catch2)
    set(CMAKE_MODULE_PATH "${Catch2_SOURCE_DIR}/contrib" ${CMAKE_MODULE_PATH})
endif()