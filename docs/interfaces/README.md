# Interface 设计索引

当前候选 interface：

- [Runtime Host C ABI v1 设计](runtime-host-c-abi.md)
- [Runtime Host C ABI v1 candidate baseline](../../include/yog_sothoth/runtime.h)
- [线程模型候选与取舍](runtime-threading-models.md)
- [Host C ABI 测试 seam](runtime-host-test-seam.md)
- [ABI candidate 验证状态](runtime-host-abi-validation.md)
- [Windows x64 布局基线](runtime-host-abi-layout-win64.json)

Status: candidate baseline

`include/yog_sothoth/runtime.h` 是唯一机器可读真源；`docs/interfaces/` 不保存第二份头文件。当前只建立 ABI 编译与布局契约，不代表 Runtime 行为已经实现。所有契约测试只允许从 Host C ABI seam 观察行为。
