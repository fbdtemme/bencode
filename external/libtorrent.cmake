include(FetchContent)

find_package(LibtorrentRasterbar QUIET)
if (LibtorrentRasterbar_FOUND)
    message(STATUS "Local installation of libtorrent-rasterbar found.")
else()
    message(STATUS "Fetching dependency libtorrent-rasterbar...")
    FetchContent_Declare(
            libtorrent-rasterbar
            GIT_REPOSITORY https://github.com/arvidn/libtorrent.git
            GIT_TAG        RC_2_0
            EXCLUDE_FROM_ALL TRUE
    )
    FetchContent_MakeAvailable(libtorrent-rasterbar)
    add_library(LibtorrentRasterbar::torrent-rasterbar ALIAS torrent-rasterbar)
endif()