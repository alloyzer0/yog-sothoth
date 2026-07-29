# 内部 implementation 通过 PRIVATE 依赖取得 C++23 要求；该 target 不安装、不导出。
add_library(yog_sothoth_internal_cpp23 INTERFACE)
target_compile_features(yog_sothoth_internal_cpp23 INTERFACE cxx_std_23)

function(ys_enable_strict_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /WX)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
endfunction()

function(ys_use_internal_cpp23 target)
    if(NOT TARGET ${target})
        message(FATAL_ERROR "ys_use_internal_cpp23 requires an existing target: ${target}")
    endif()

    # PRIVATE 防止内部语言标准要求传播到 Host 或未来安装导出的 target。
    target_link_libraries(${target} PRIVATE yog_sothoth_internal_cpp23)
    set_target_properties(${target} PROPERTIES CXX_EXTENSIONS OFF)
endfunction()
