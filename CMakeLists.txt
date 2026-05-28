cmake_minimum_required(VERSION 3.18)
project(CacheServer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${PROJECT_SOURCE_DIR}/include)

set(SOURCES
    ${PROJECT_SOURCE_DIR}/src/cache.cpp
    ${PROJECT_SOURCE_DIR}/src/handler.cpp
    ${PROJECT_SOURCE_DIR}/src/main.cpp
    ${PROJECT_SOURCE_DIR}/src/protocol.cpp
    ${PROJECT_SOURCE_DIR}/src/server.cpp
)

# 检查源文件是否存在
foreach(source_file ${SOURCES})
    if(NOT EXISTS ${source_file})
        message(FATAL_ERROR "Source file does not exist: ${source_file}")
    endif()
endforeach()

add_executable(cache_server ${SOURCES})

target_link_libraries(cache_server pthread)