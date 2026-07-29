# ADR-0001：Host/Output C ABI 与线程模型

Status: accepted for Phase 1

## Context

Yog-Sothoth 需要被 Viewer、Headless CLI、未来游戏/编辑器和语言 SDK 使用。若公共 interface 暴露 Vulkan、C++ ABI 或窗口专用概念，宿主集成和长期兼容会被底层实现锁定。

## Decision

- 以 C17 ABI 作为唯一稳定外部 seam，使用 opaque handle、显式 ABI version 和 `struct_size`；
- Runtime 对外核心概念为 Runtime、AssetPackage、FeatureProfile、SceneTransaction、SceneVersion、View、Output、FrameTicket；
- Host 与 Output 分离；Phase 1 实现 Viewer/Headless Host Adapter 和 Swapchain/Offscreen Readback Output Adapter；
- `render` 非阻塞，GPU 完成通过 FrameTicket 查询；
- Runtime 拥有内部 GPU 对象，Host 不持有 Vulkan handle；
- Phase 1 的外部调用串行进入 Runtime；内部可使用工作/提交线程；
- Runtime 不从未声明线程调用 Host 回调；完成通知优先使用轮询 ticket。
- Phase 1 的正式 Progress Driver 为 Host-pumped；Runtime command intake、Runtime Reactor 和 Progress Driver 必须分离；
- Host ABI v1 在 Validation/Vulkan Adapter、HostPump/DedicatedDriver 和关键对象生命周期经过真实实现验证前保持 candidate，不承诺向后兼容；冻结前允许伴随契约、layout 基线和 ADR 一起进行破坏性修订。
- Host ABI v1 冻结前，用 Validation Adapter 的最小 DedicatedDriver spike 复用同一套 Host ABI 契约测试，验证未来内部线程模式不会改动外部 interface 和核心状态机。
- SceneVersion 与 FrameTicket 的 Host 引用可从任意 Host 线程释放；其他 mutation/destroy 保持 control/owner thread；
- 跨线程 release 进入并发 Release Inbox，实际回收和 GPU retirement 仍由 Runtime Reactor 串行执行。
- v1 不提供阻塞式 `wait_shutdown`；关闭流程固定为 shutdown、显式 poll/get_state、STOPPED 后 destroy；
- 同步错误使用 caller-owned 结构化 `ys_error_info`；文本写入 Host 提供的可选 UTF-8 缓冲，完整异步/长期证据通过 diagnostics id 查询；
- Offscreen readback 仅绑定 FrameTicket 的只读 map/unmap lease；ticket 保存不可变输出元数据，Runtime 不承担图片编码或文件 I/O；
- v1 不提供 `frame_wait`；Host-pumped 使用 poll/query，未来跨线程消费者或 Dedicated 成为真实能力后再新增只等待、不推进的 wait interface；
- 所有扩展结构通过 `structure_type` 识别；capability 集合使用独立 bit flags，不复用 kind 枚举；
- Output destroy 是逻辑销毁，立即拒绝新 render 并停止新的 acquire/present；在途 revision/readback/report 由 FrameTicket 延迟退休；
- Win32 Swapchain 的 HWND/HINSTANCE 由 Host 拥有，从 Output create 到 destroy 返回期间保持有效；destroy 返回后 Host 可以销毁窗口，在途 present 失败通过 ticket 报告；
- FeatureProfile 是 Host 产品配置 seam，不暴露 pass/resource/barrier；Profile revision 和 package 引用由 Runtime 捕获并延迟退休；
- FrameTicket 通过分类 JSON report 暴露 workload、timings、metrics、resources 和 provenance 证据；这是只读观测，不是 Workload IR interface；
- Frame report 由每帧 capture flags 显式启用；默认不采集昂贵 timestamp/resource/workload trace；
- DLL 符号固定 YS_API/YS_CALL，ABI major 通过无 Runtime 的查询入口协商，record 使用变长字节流；
- Runtime 创建前可枚举 device/surface 能力；Swapchain 明确 acquire/present 与 OUTDATED/SUBOPTIMAL/SURFACE_LOST 状态；
- execution capacity 与 retained-result/readback capacity 分离，证据采集不反向阻塞 GPU 执行槽；

## Consequences

- C++、Rust、Zig、C#、Python 等 SDK 只能包装 C ABI，不能复制执行逻辑；
- 外部纹理互操作未来需要版本化扩展结构，不能直接污染基础 interface；
- C ABI 只约束 Host 操作，内部 Feature 和 GPU 实现可以演进；
- 需要契约测试验证结构体扩展、错误传播、生命周期和重入规则。
- DedicatedDriver spike 只验证架构，不构成 Phase 1 对 Vulkan Dedicated 模式的产品承诺。
- Runtime destroy 前，Host 必须停止并 join 自己的调用线程；destroy 不与并发 ABI 调用竞态。
- Dedicated 成为真实 Adapter 后可以新增 wait interface，但不得改变现有 shutdown/destroy 语义。
- ABI 不内嵌固定大小错误字符串，也不依赖 TLS last-error 或 Runtime-owned 临时字符串。
- active readback lease 阻止 ticket release 和 Runtime STOPPED；Host 必须先 unmap，且 unmap 与 map 在同一线程。
- 未来 frame wait 不得隐式推进 Runtime Reactor，也不得允许 Host-pumped control thread 对未完成 ticket 阻塞。
- Runtime 不处理 Win32 消息或销毁窗口；surface lost 通过结构化错误要求 Host 重建 Output。

## Rejected

- 公共 C++ interface：ABI 和工具链耦合过强；
- 公开 Vulkan 设备/队列：破坏 Runtime 的同步与所有权；
- 将窗口/交换链绑定到 Runtime：阻断 Headless、共享纹理和编码输出。
