include(FetchContent)

find_package(libtorrent QUIET)
if (libtorrent_FOUND)
    message(STATUS "Local installation of libtorrent found.")
else()
    message(STATUS "Fetching dependency libtorrent...")
    FetchContent_Declare(
            libtorrent
            GIT_REPOSITORY https://github.com/arvidn/libtorrent.git
            GIT_TAG        RC_2_0
    )
    FetchContent_MakeAvailable(libtorrent)
    add_library(LibtorrentRasterbar::torrent-rasterbar ALIAS torrent-rasterbar)
endif()