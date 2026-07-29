# ADR-0005：内部 C++23 语言基线与 C ABI 隔离

Status: accepted for Phase 1

## Context

Yog-Sothoth 需要使用现代 C++ 构建高性能 Runtime，同时作为可嵌入库被不同语言、编译器和宿主工具链调用。若把内部语言标准作为公共依赖传播，Host 会被迫跟随 Runtime 升级；若全局设置 C++23，又会使 C++20 Host 兼容性测试失真。

## Decision

- 所有 Runtime 内部 C++ implementation target 以标准 C++23 为最低语言模式，并关闭编译器语言扩展；
- C++23 要求通过非安装、非导出的 `yog_sothoth_internal_cpp23` CMake target 以 `PRIVATE` 依赖施加；
- 不设置全局 `CMAKE_CXX_STANDARD`，公共 Host interface 不继承内部语言标准；
- 唯一稳定外部 seam 继续是 C17 ABI；公共头文件必须通过 C17、C++20 和 C++23 Host 编译测试；
- C ABI 不暴露 C++ class、标准库类型、异常、RTTI 对象或编译器相关布局；
- 内部可以使用 C++ 异常，但所有 C ABI 入口必须捕获并映射为 `ys_result`，异常不得逃出 ABI；
- Runtime 分配的对象由 Runtime 释放，不允许跨模块混用 C++ allocator 或标准库容器所有权；
- 内部 C++ ABI 不承诺稳定；同一 Runtime 二进制及其内部 module 必须使用兼容工具链和 C++ runtime 构建；
- C++23 标准库能力按实际使用通过 feature probe 引入，不因进入 C++23 模式就假定全部库特性可用。

## Consequences

- Scene、Workload、Asset、GPU 和 Diagnostics module 可以使用 C++23 implementation 能力，而 Host 无需升级语言标准；
- C++20 Host 测试必须保留，C++23 Host 测试是附加覆盖而不是替代；
- 每个新增内部 C++ target 必须通过 `ys_use_internal_cpp23` 接入构建策略；
- MSVC、Clang-cl 和其他工具链必须运行相同模式测试，未运行时只能标记为未验收；
- 未来若提供 C++ convenience wrapper，应放在独立头文件中，并单独决定调用方语言基线。

## Rejected

- 全局设置 `CMAKE_CXX_STANDARD 23`：会污染 Host 兼容性测试并把内部要求传播到无关 target；
- 把公共 Host interface 升级为 C++23：破坏 C/FFI 和旧宿主工具链集成；
- 在 C ABI 中暴露 STL 类型或 C++ exception：形成编译器和运行库耦合；
- 仅依据 `-std=c++23` 可用就假定完整标准库支持：无法表达不同工具链的 feature 差异。
