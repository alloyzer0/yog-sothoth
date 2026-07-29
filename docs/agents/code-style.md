# C/C++ 代码风格

本文规定仓库内 C17 Host interface、C 测试和内部 C++23 implementation 的统一风格。

## 格式化真源

- 仓库根目录的 `.clang-format` 是格式化规则的唯一真源；
- 使用 clang-format 18 或更高版本，禁止手工维护与格式化器冲突的空格或换行；
- 修改 `include/`、`src/` 或 `tests/` 下的 C/C++ 文件后运行 `format-check`；
- 只有机器可读 ABI 宏、生成代码或工具误判时才能局部使用 `clang-format off/on`，并说明原因；
- Host C ABI 初始化宏的字段分组由人工维护，`.clang-format` 不重排宏体。

配置并运行开发目标：

```sh
cmake -S . -B build/dev -DBUILD_TESTING=ON
cmake --build build/dev --target format
cmake --build build/dev --target format-check
```

## 格式摘要

- 四空格缩进，不使用 Tab；
- 行宽上限为 100 列；
- 函数定义的左花括号独占下一行；控制语句、类型和 namespace 的左花括号保留在声明行；
- 指针与引用符号靠近类型，例如 `const AdvanceBudget& budget`；
- 构造函数初始化列表从下一行开始，每个初始化项独立排列；
- 换行后的函数实参和形参不进行 bin packing；
- 不自动重排注释或 Host C ABI 初始化宏体。

## 命名

- 类型使用 `PascalCase`；函数、变量和 namespace 使用 `snake_case`；
- 私有数据成员使用尾随下划线，例如 `control_thread_`；
- 内部只读、无副作用且低成本的属性查询使用名词，例如 `phase()`、`state()`，不添加 `get_`；
- 布尔查询优先使用 `is_`、`has_` 或 `can_`；改变状态的操作使用动词；
- 公共 C ABI 使用 `ys_<object>_<operation>`，查询操作可以使用 `get` 或 `query` 明确语义；
- 领域命名遵循根目录 `CONTEXT.md`，不得使用其中明确要求避免的同义词。

clang-format 只约束布局，不校验命名；命名规则由 review 和后续静态检查共同保证。
