include(FetchContent)

find_package(expected-lite QUIET)
if (expected-lite_FOUND )
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