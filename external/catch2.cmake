
find_package(Catch2 QUIET)
if (Catch2_FOUND)
    message(STATUS "Local installation of Catch2 found.")
else()
    message(STATUS "Fetching dependency Catch2...")
    FetchContent_Declare(
            Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG        v2.x
            EXCLUDE_FROM_ALL TRUE
    )
    FetchContent_MakeAvailable(Catch2)
    set(CMAKE_MODULE_PATH "${Catch2_SOURCE_DIR}/contrib" ${CMAKE_MODULE_PATH})
endif()