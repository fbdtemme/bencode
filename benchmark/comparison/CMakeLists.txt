cmake_minimum_required(VERSION 3.15)


project("bencode-comparision-benchmark")

include(CTest)
include(Catch)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
        PRIVATE
        main.cpp
        fbdtemme_bencode.cpp
        jimporter_bencode.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE external)



target_compile_definitions(${PROJECT_NAME} PRIVATE RESOURCES_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/../resources\")
target_link_libraries(${PROJECT_NAME} bencode::bencode Catch2::Catch2)
target_compile_options(${PROJECT_NAME}  PUBLIC)

catch_discover_tests(${PROJECT_NAME})
