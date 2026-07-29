function(ys_add_clang_format_targets)
    find_program(
        YS_CLANG_FORMAT_EXECUTABLE
        NAMES
            clang-format-22
            clang-format-21
            clang-format-20
            clang-format-19
            clang-format-18
            clang-format
        DOC "clang-format executable used by Yog-Sothoth developer targets"
    )

    if(NOT YS_CLANG_FORMAT_EXECUTABLE)
        message(STATUS "clang-format 18 or newer not found; format targets are unavailable")
        return()
    endif()

    execute_process(
        COMMAND "${YS_CLANG_FORMAT_EXECUTABLE}" --version
        OUTPUT_VARIABLE ys_clang_format_version_output
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE ys_clang_format_version_result
    )
    if(NOT ys_clang_format_version_result EQUAL 0)
        message(WARNING "Unable to query clang-format; format targets are unavailable")
        return()
    endif()

    string(
        REGEX MATCH
        "version[ \t]+([0-9]+)"
        ys_clang_format_version_match
        "${ys_clang_format_version_output}"
    )
    if(NOT ys_clang_format_version_match)
        message(WARNING "Unable to parse clang-format version: ${ys_clang_format_version_output}")
        return()
    endif()
    if(CMAKE_MATCH_1 LESS 18)
        message(
            WARNING
            "clang-format 18 or newer is required; found ${ys_clang_format_version_output}"
        )
        return()
    endif()

    file(
        GLOB_RECURSE
        ys_clang_format_sources
        CONFIGURE_DEPENDS
        LIST_DIRECTORIES false
        "${PROJECT_SOURCE_DIR}/include/*.c"
        "${PROJECT_SOURCE_DIR}/include/*.h"
        "${PROJECT_SOURCE_DIR}/include/*.cpp"
        "${PROJECT_SOURCE_DIR}/include/*.hpp"
        "${PROJECT_SOURCE_DIR}/src/*.c"
        "${PROJECT_SOURCE_DIR}/src/*.h"
        "${PROJECT_SOURCE_DIR}/src/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/*.hpp"
        "${PROJECT_SOURCE_DIR}/tests/*.c"
        "${PROJECT_SOURCE_DIR}/tests/*.h"
        "${PROJECT_SOURCE_DIR}/tests/*.cpp"
        "${PROJECT_SOURCE_DIR}/tests/*.hpp"
    )
    list(SORT ys_clang_format_sources)

    add_custom_target(
        format
        COMMAND
            "${YS_CLANG_FORMAT_EXECUTABLE}"
            --style=file
            --fallback-style=none
            -i
            ${ys_clang_format_sources}
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        COMMENT "Formatting Yog-Sothoth C and C++ sources"
        COMMAND_EXPAND_LISTS
        VERBATIM
    )

    add_custom_target(
        format-check
        COMMAND
            "${YS_CLANG_FORMAT_EXECUTABLE}"
            --style=file
            --fallback-style=none
            --dry-run
            --Werror
            ${ys_clang_format_sources}
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        COMMENT "Checking Yog-Sothoth C and C++ source formatting"
        COMMAND_EXPAND_LISTS
        VERBATIM
    )

    message(STATUS "clang-format targets use ${ys_clang_format_version_output}")
endfunction()
