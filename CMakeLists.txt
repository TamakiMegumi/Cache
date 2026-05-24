cmake_minimum_required(VERSION 3.18)
project(CacheServer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

set(SOURCES
    ${PROJECT_SOURCE_DIR}/src/*.cpp
)

add_executable(cache_server ${SOURCES})

target_link_libraries(cache_server pthread)


