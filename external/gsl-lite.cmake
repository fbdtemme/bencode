include(FetchContent)

find_package(gsl-lite QUIET)
if (gsl-lite_FOUND )
    message(STATUS "Local installation of gsl-lite found.")
else()
    message(STATUS "Fetching dependency gsl-lite...")
    FetchContent_Declare(
            gsl-lite
            GIT_REPOSITORY https://github.com/gsl-lite/gsl-lite.git
            GIT_TAG        master
    )
    FetchContent_MakeAvailable(gsl-lite)
endif()