add_library(
    yog_sothoth_runtime
    STATIC
        host_c_api.cpp
        runtime_reactor.cpp
        runtime_core.cpp
)

target_link_libraries(
    yog_sothoth_runtime
    PUBLIC
        yog_sothoth_runtime_headers
)

# 当前 Ticket 先验证静态嵌入形态；动态库使用同一 C ABI，后续单独验收。
target_compile_definitions(yog_sothoth_runtime PUBLIC YS_STATIC)
ys_use_internal_cpp23(yog_sothoth_runtime)
ys_enable_strict_warnings(yog_sothoth_runtime)
