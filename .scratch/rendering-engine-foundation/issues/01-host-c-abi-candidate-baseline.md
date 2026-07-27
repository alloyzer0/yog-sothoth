# 建立 Host C ABI candidate baseline 与跨语言编译测试

Status: ready-for-agent
Resolution: completed
Blocked by:

## Goal

建立 CMake/C++20 工程骨架，把 Runtime Host C ABI 发布为唯一公共头文件，并用 C17、C++20 和 ABI layout 契约测试固定候选基线。本 issue 不实现任何 Runtime 函数。

## Test seam

唯一测试 seam 是 `include/yog_sothoth/runtime.h`。测试不得包含私有 C++ 类型或 Vulkan 头文件。

## Acceptance

- CMake + CTest 工程可用；
- 公共头文件可由 C17 和 C++20 严格编译；
- `YS_STATIC`、`YS_BUILD_DLL`、默认导入三种宏路径可编译；
- ABI layout 测试固定公共结构的 `sizeof`、`alignof` 和关键 `offsetof`；
- MinGW 验证通过；
- MSVC、Clang-cl 若不可用，明确记录为未验收；
- `docs/interfaces/` 只链接唯一公共头文件，不保留第二份真源；
- 不提供 Runtime 实现或链接期假成功 stub。

## Comments

- 2026-07-25：完成 CMake/Ninja/CTest 骨架；公共头文件移动到 `include/yog_sothoth/runtime.h`，docs 只保留链接。
- MinGW GCC/G++ 14.2.0 的 C17、C++20、静态/导出/导入宏路径与 Windows x64 ABI layout 测试通过。
- MSVC 与 Clang-cl 当前不可用，已在验证状态文档和机器可读布局基线中标记为未验收。
- 未添加任何 Runtime 实现或链接期 stub。
