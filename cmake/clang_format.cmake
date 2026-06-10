function(add_clang_format_target target)
    if(NOT CLANG_FORMAT_EXECUTABLE)
        unset(CLANG_FORMAT_EXECUTABLE CACHE)
        set(clang_format_hints)

        if(WIN32)
            list(APPEND clang_format_hints
                "$ENV{ProgramFiles}/LLVM/bin"
                "$ENV{ProgramFiles\(x86\)}/LLVM/bin")

            file(GLOB clang_format_visual_studio_paths
                "$ENV{ProgramFiles}/Microsoft Visual Studio/*/*/VC/Tools/Llvm/x64/bin")
            list(APPEND clang_format_hints ${clang_format_visual_studio_paths})
        endif()

        find_program(CLANG_FORMAT_EXECUTABLE
            NAMES clang-format clang-format-22
            HINTS ${clang_format_hints}
            DOC "Path to the clang-format executable")
    endif()

    if(NOT CLANG_FORMAT_EXECUTABLE)
        message(WARNING
            "clang-format was not found; the 'format' target will not be available. "
            "Set CLANG_FORMAT_EXECUTABLE to its full path.")
        return()
    endif()

    get_target_property(target_sources ${target} SOURCES)
    list(FILTER target_sources INCLUDE REGEX "\\.(c|cc|cpp|cxx|h|hh|hpp|hxx)$")

    set(source_files)
    foreach(source IN LISTS target_sources)
        if(IS_ABSOLUTE "${source}")
            list(APPEND source_files "${source}")
        else()
            list(APPEND source_files "${CMAKE_CURRENT_SOURCE_DIR}/${source}")
        endif()
    endforeach()

    add_custom_target(format
        COMMAND ${CLANG_FORMAT_EXECUTABLE} -i --style=file ${source_files}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Formatting C/C++ source files"
        VERBATIM)
endfunction()
