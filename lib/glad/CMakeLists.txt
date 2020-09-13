cmake_minimum_required(VERSION 3.13)

set(HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/glad/glad.h" "${CMAKE_CURRENT_SOURCE_DIR}/include/KHR/khrplatform.h")
set(SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/glad.c")

add_library(glad INTERFACE IMPORTED GLOBAL )
target_sources(glad INTERFACE ${HEADERS} ${SOURCES})
target_include_directories(glad INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include/")

if(UNIX)
    target_link_libraries (glad INTERFACE dl)
endif()