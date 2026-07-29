add_executable(internal_cpp23_compile cpp23_internal_compile.cpp)

# 该测试只观察内部构建策略，不包含任何 Runtime 私有类型或行为。
ys_use_internal_cpp23(internal_cpp23_compile)
ys_enable_strict_warnings(internal_cpp23_compile)

add_test(NAME toolchain.cpp23.internal COMMAND internal_cpp23_compile)
