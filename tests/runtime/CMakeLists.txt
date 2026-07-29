add_executable(runtime_lifecycle lifecycle.c)

# 测试只通过公共 Host C ABI 观察行为；Runtime target 由 implementation 提供。
target_link_libraries(
    runtime_lifecycle
    PRIVATE
        yog_sothoth_runtime_headers
        yog_sothoth_runtime
)
target_compile_features(runtime_lifecycle PRIVATE c_std_17)
set_target_properties(runtime_lifecycle PROPERTIES C_EXTENSIONS OFF)
target_compile_definitions(runtime_lifecycle PRIVATE YS_STATIC)
ys_enable_strict_warnings(runtime_lifecycle)

add_test(NAME runtime.lifecycle COMMAND runtime_lifecycle)
set_tests_properties(runtime.lifecycle PROPERTIES LABELS "runtime;lifecycle")
