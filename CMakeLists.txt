cmake_minimum_required(VERSION 3.14)
project(Bencode
        DESCRIPTION "A modern C++ Bencode library"
        LANGUAGES CXX
        VERSION 0.1.0)

set(PROJECT_AUTHOR "fbdtemme")
# Make Find modules in cmake dir available
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# includes
include(CTest)
include(CMakeDependentOption)
include(GNUInstallDirs)
include(SanitizersConfig)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(BENCODE_STANDALONE TRUE)
endif()


add_library(bencode INTERFACE)
target_compile_features(bencode INTERFACE cxx_std_20)

target_include_directories(bencode INTERFACE
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        )

include(external/CMakeLists.txt)


cmake_dependent_option(BENCODE_BUILD_TESTS    "Build tests."
                       ON  "BUILD_TESTING;BENCODE_STANDALONE" OFF)
option(BENCODE_BUILD_TESTS_COVERAGE   "Build tests with coverage" OFF)

cmake_dependent_option(BENCODE_BUILD_BENCHMARKS  "Build benchmarks"
                       ON  "BENCODE_STANDALONE"      OFF)

cmake_dependent_option(BENCODE_BUILD_DOCS       "Build documentation"
                        ON  "BENCODE_STANDALONE"  OFF)

option(BENCODE_ENABLE_COVERAGE "Build with coverage enabled" ON)

# Concepts compile flags

target_link_libraries(bencode
    INTERFACE
        fmt::fmt
        Microsoft.GSL::GSL
        nonstd::expected-lite)

add_library(bencode::bencode ALIAS bencode)


if (BENCODE_BUILD_TESTS)
    enable_testing()
    message(STATUS "Building tests enabled")
    add_subdirectory(tests)
endif()


if (BENCODE_BUILD_BENCHMARKS)
    message(STATUS "Building benchmark enabled")
    add_subdirectory(benchmark)
endif()

if (BENCODE_BUILD_DOCS)
    message(STATUS "Building docs enabled")
    add_subdirectory(docs)
endif()

if (NOT fmt_FOUND)
    message(STATUS "Adding fmt to install set")
    install(TARGETS fmt
            EXPORT bencode-targets
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()

if (NOT expected-lite_FOUND)
    message(STATUS "Adding expected-lite to install set")
    install(TARGETS expected-lite
            EXPORT bencode-targets
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()

if (NOT Microsoft.GSL_FOUND)
    message(STATUS "Adding Microsoft.GSL to install set")
    install(TARGETS GSL
            EXPORT bencode-targets
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()



# install targets to an export set
install(TARGETS bencode
        EXPORT bencode-targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})



# Install the export set
install(EXPORT bencode-targets
        FILE bencode-targets.cmake
        NAMESPACE bencode::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/bencode)
