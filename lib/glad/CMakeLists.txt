add_library(glad STATIC "${CMAKE_CURRENT_SOURCE_DIR}/include/glad/glad.h" "${CMAKE_CURRENT_SOURCE_DIR}/include/KHR/khrplatform.h"
"${CMAKE_CURRENT_SOURCE_DIR}/src/glad.c")

target_include_directories(glad SYSTEM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")

if(UNIX)
    target_link_libraries (glad PUBLIC dl)
endif()
