cmake_minimum_required(VERSION 3.19)
project(ucomposer)

find_package(CURL)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -march=x86-64")

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        "src/*.h"
        "src/*.hpp"
        "src/*.cpp"
)

add_executable(
        ucomposer
        main.cpp
        ${SOURCES}
)


target_include_directories(
        ucomposer
        PUBLIC
        3dparty
        src
)

target_link_libraries(
        ucomposer
        CURL::libcurl
)