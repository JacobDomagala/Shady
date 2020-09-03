cmake_minimum_required(VERSION 3.13)

set(HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/glad/glad.h" "${CMAKE_CURRENT_SOURCE_DIR}/include/KHR/khrplatform.h")
set(SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/glad.c")

add_library(GLAD INTERFACE IMPORTED GLOBAL )
target_sources(GLAD INTERFACE ${HEADERS} ${SOURCES})
target_include_directories(GLAD INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include/")
# For linux we may need to add -ldl
#target_link_libraries (GLAD INTERFACE opengl32)