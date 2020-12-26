include(FetchContent)

find_package(fmt QUIET)
if (fmt_FOUND)
    message(STATUS "Local installation of fmt found.")
else()
    message(STATUS "Fetching dependency fmt...")
    FetchContent_Declare(
            fmt
            GIT_REPOSITORY https://github.com/fmtlib/fmt.git
            GIT_TAG        master
            EXCLUDE_FROM_ALL TRUE
    )
    set(FMT_INSTALL ON)
    set(BUILD_SHARED_LIBS ON)
    FetchContent_MakeAvailable(fmt)
endif()

