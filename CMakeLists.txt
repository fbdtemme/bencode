cmake_minimum_required(VERSION 3.14)
project(bencode
        DESCRIPTION "A modern C++ Bencode library"
        HOMEPAGE_URL https://github.com/fbdtemme/bencode
        LANGUAGES CXX
        VERSION 0.2.0)

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
add_library(bencode::bencode ALIAS bencode)

target_compile_features(bencode INTERFACE cxx_std_20)

target_include_directories(bencode INTERFACE
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        )

include(external/external.cmake)


option(BENCODE_BUILD_TESTS    "Build tests."  ON)
cmake_dependent_option(BENCODE_BUILD_TESTS_COVERAGE
                       "Build with coverage enabled"
                       OFF "BENCODE_BUILD_TESTS" OFF)

option(BENCODE_BUILD_BENCHMARKS  "Build benchmarks"    OFF)
option(BENCODE_BUILD_DOCS        "Build documentation" OFF)

# Concepts compile flags

target_link_libraries(bencode
    INTERFACE
        fmt::fmt-header-only
        Microsoft.GSL::GSL
        nonstd::expected-lite)

add_library(bencode::bencode ALIAS bencode)


if ((BUILD_TESTING AND BENCODE_BUILD_TESTS) OR BENCODE_BUILD_TESTS)
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

set(bencode_cmake_install_dir   ${CMAKE_INSTALL_LIBDIR}/cmake/bencode)
set(bencode_version_config      ${PROJECT_BINARY_DIR}/bencode-config-version.cmake)
set(bencode_project_config      ${PROJECT_BINARY_DIR}/bencode-config.cmake)
set(bencode_targets_export_name bencode-targets)
set(bencode_targets_file        ${bencode_targets_export_name}.cmake)
set(bencode_include_build_dir   ${PROJECT_SOURCE_DIR}/include/)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
        ${bencode_version_config}
        VERSION ${PACKAGE_VERSION}
        COMPATIBILITY AnyNewerVersion
)
configure_package_config_file(
        ${PROJECT_SOURCE_DIR}/cmake/bencode-config.cmake.in
        ${bencode_project_config}
        INSTALL_DESTINATION ${bencode_cmake_install_dir})


# install headers
install(DIRECTORY ${bencode_include_build_dir}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
# install project config file and config version file
install(FILES ${bencode_project_config}
              ${bencode_version_config}
        DESTINATION ${bencode_cmake_install_dir}
)


# install targets to an export set
install(TARGETS bencode
        EXPORT ${bencode_targets_export_name}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})


if (NOT fmt_FOUND)
    message(STATUS "Adding fmt to install set")
    install(TARGETS fmt fmt-header-only
            EXPORT ${bencode_targets_export_name}
            INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()

if (NOT expected-lite_FOUND)
    message(STATUS "Adding expected-lite to install set")
    install(TARGETS expected-lite
            EXPORT ${bencode_targets_export_name}
            INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()

if (NOT Microsoft.GSL_FOUND)
    message(STATUS "Adding Microsoft.GSL to install set")
    install(TARGETS GSL
            EXPORT ${bencode_targets_export_name}
            INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()


# Install the export set to enable importing targets from the build tree
export(EXPORT ${bencode_targets_export_name}
        FILE ${bencode_targets_file}
        NAMESPACE ${PROJECT_NAME}::)

# Install the export set to enable importing targets from the install tree
install(EXPORT ${bencode_targets_export_name}
        FILE ${bencode_targets_file}
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${bencode_cmake_install_dir})
