# ADR-0001：Host/Output C ABI 与线程模型

Status: accepted for Phase 1

## Context

Yog-Sothoth 需要被 Viewer、Headless CLI、未来游戏/编辑器和语言 SDK 使用。若公共 interface 暴露 Vulkan、C++ ABI 或窗口专用概念，宿主集成和长期兼容会被底层实现锁定。

## Decision

- 以 C17 ABI 作为唯一稳定外部 seam，使用 opaque handle、显式 ABI version 和 `struct_size`；
- Runtime 对外核心概念为 Runtime、SceneTransaction、SceneVersion、View、Output、FrameTicket；
- Host 与 Output 分离；Phase 1 实现 Viewer/Headless Host Adapter 和 Swapchain/Offscreen Readback Output Adapter；
- `render` 非阻塞，GPU 完成通过 FrameTicket 查询；
- Runtime 拥有内部 GPU 对象，Host 不持有 Vulkan handle；
- Phase 1 的外部调用串行进入 Runtime；内部可使用工作/提交线程；
- Runtime 不从未声明线程调用 Host 回调；完成通知优先使用轮询 ticket。

## Consequences

- C++、Rust、Zig、C#、Python 等 SDK 只能包装 C ABI，不能复制执行逻辑；
- 外部纹理互操作未来需要版本化扩展结构，不能直接污染基础 interface；
- C ABI 只约束 Host 操作，内部 Feature 和 GPU 实现可以演进；
- 需要契约测试验证结构体扩展、错误传播、生命周期和重入规则。

## Rejected

- 公共 C++ interface：ABI 和工具链耦合过强；
- 公开 Vulkan 设备/队列：破坏 Runtime 的同步与所有权；
- 将窗口/交换链绑定到 Runtime：阻断 Headless、共享纹理和编码输出。

