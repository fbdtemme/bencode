include(FetchContent)


find_package(Boost REQUIRED COMPONENTS regex)

message(STATUS "Fetching dependency kriben/bencode...")
FetchContent_Declare(
        kriben-bencode
        GIT_REPOSITORY https://github.com/kriben/bencode.git
        GIT_TAG        master
)
FetchContent_MakeAvailable(kriben-bencode)
file(GLOB kriben-bencode_SOURCES "${kriben-bencode_SOURCE_DIR}/*.cpp")

add_library(kriben-bencode SHARED ${kriben-bencode_SOURCES})
target_link_libraries(kriben-bencode PUBLIC Boost::headers Boost::regex)
target_include_directories(kriben-bencode PUBLIC ${kriben-bencode_SOURCE_DIR})