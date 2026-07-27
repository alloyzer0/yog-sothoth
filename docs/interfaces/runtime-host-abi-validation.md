# Runtime Host ABI candidate 验证状态

Status: candidate baseline

## 唯一公共头文件

[include/yog_sothoth/runtime.h](../../include/yog_sothoth/runtime.h) 是机器可读 ABI 的唯一真源。设计文档不保存头文件副本。

## 当前已验收

- Windows x64；
- MinGW GCC/G++ 14.2.0；
- C17 严格编译：`-Wall -Wextra -Wpedantic -Werror`；
- C++20 严格编译；
- `YS_STATIC`、`YS_BUILD_DLL` 和默认 import 声明路径；
- 关键结构的 `sizeof`、`alignof`、`offsetof`；
- CMake + Ninja + CTest。

机器可读基线：[runtime-host-abi-layout-win64.json](runtime-host-abi-layout-win64.json)

## 尚未验收

- MSVC：当前环境未发现 `cl.exe`；
- Clang-cl：当前环境未发现 `clang-cl.exe`。

在 ABI v1 从 candidate 升为 frozen 前，必须让 MSVC 与 Clang-cl 运行同一布局和编译契约测试。未验收不等于失败，也不得宣称跨编译器布局已确认。

## 命令

```powershell
cmake --preset mingw-debug
cmake --build --preset mingw-debug
ctest --preset mingw-debug
```

这些测试只验证公共头文件和布局；仓库尚未提供任何 Runtime 函数实现。
