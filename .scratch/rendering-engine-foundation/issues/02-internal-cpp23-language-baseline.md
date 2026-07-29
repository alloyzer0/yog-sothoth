# 建立内部 C++23 语言与工具链基线

Type: task
Status: ready-for-agent
Resolution: completed
Blocked by: 01

## Goal

将所有 Runtime 内部 C++ implementation target 的语言基线设为标准 C++23，同时保持公共 Host C ABI 为 C17，并继续验证 C++20 Host 可消费公共头文件。本 issue 不实现任何 Runtime 行为。

## Test seams

- 外部 seam：`include/yog_sothoth/runtime.h`，分别由 C17、C++20 和 C++23 translation unit 编译；
- 内部 seam：非导出的 CMake target `yog_sothoth_internal_cpp23`，所有内部 implementation target 通过 `PRIVATE` 依赖取得 C++23 编译要求。

## Acceptance

- 不设置全局 `CMAKE_CXX_STANDARD`，避免污染 Host 兼容性测试；
- 内部 CMake 策略要求 `cxx_std_23` 且禁用编译器语言扩展；
- C17、C++20 Host 编译测试继续通过；
- 新增 C++23 Host 编译测试；
- 新增内部 C++23 模式契约测试；
- Windows x64 ABI layout 基线不变；
- 更新 Phase 1 规格、C4、ABI 验证记录并新增 ADR；
- MinGW 验证通过，MSVC 和 Clang-cl 保持明确的未验收状态；
- 不提供 Runtime 实现或链接期假成功 stub。

## Comments

- 2026-07-27：用户确认执行 Ticket 2；先以内部工具链契约建立红灯测试。
- 红灯 1：内部工具链测试在默认 C++17 下失败，`__cplusplus` 为 `201703L`；加入私有 C++23 构建策略后通过。
- 红灯 2：C++23 Host 测试在未声明语言标准时以默认 C++17 失败；加入 target 级 `cxx_std_23` 后通过。
- MinGW clean build 显式使用 `-std=c++20` 编译 C++20 Host，并使用 `-std=c++23` 编译 C++23 Host 与内部工具链测试。
- CTest 5/5 通过；ABI layout 的 16 个采样字段与 Windows x64 基线一致；安装产物仍只有公共 `runtime.h`。
- MSVC 与 Clang-cl 当前仍未验收；未添加任何 Runtime 行为、符号 stub 或私有实现类型。
