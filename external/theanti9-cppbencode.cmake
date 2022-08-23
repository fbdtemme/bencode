include(FetchContent)

message(STATUS "Fetching dependency theanti9/cppbencode...")
FetchContent_Declare(
        theanti9-cppbencode
        GIT_REPOSITORY https://github.com/theanti9/cppbencode.git
        GIT_TAG        master
)
FetchContent_MakeAvailable(theanti9-cppbencode)
file(GLOB theanti9-cppbencode_SOURCES "${theanti9-cppbencode_SOURCE_DIR}/*.cc")

add_library(theanti9-cppbencode STATIC ${theanti9-cppbencode_SOURCES})
target_include_directories(theanti9-cppbencode PUBLIC ${theanti9-cppbencode_SOURCE_DIR})
