# Yog-Sothoth 学习与实现路线

Status: active
Updated: 2026-07-29

## 项目定位

Yog-Sothoth 首先是学习驱动、研究导向的渲染 Runtime。项目使用生产级工程纪律约束正确性、生命周期、性能和证据，但不以短期功能完整度或外部产品稳定性作为首要成功指标。

当前 `spec.md` 保留长期能力设计和候选契约；本文是当前执行顺序与阶段完成条件的真源，并取代 `spec.md` 第 15、16 节原有的执行排序与 Phase 1 完成定义。

## 执行原则

- 保持窄范围、端到端闭环，不横向铺设完整引擎模块；
- 保持 `WIP = 1`，每个 slice 完成契约、实现、review 和复盘后再推进；
- 代码结构按真实状态、不变量、seam 和第二种 implementation 演进提取；
- Host C ABI 在 Validation/Vulkan、HostPump/DedicatedDriver 和关键生命周期验证前保持 candidate；
- 性能先满足架构不变量，再通过固定实验建立数值目标；
- 论文先独立复现并与传统基线比较，再通过既有 seam 接入。

## Learning Epochs

### Epoch A：Runtime Foundation

建立 Runtime 生命周期、C ABI 异常边界、结构化错误、control thread 规则和 Host-pumped 推进语义。

完成条件：

- `create → RUNNING → shutdown → QUIESCING → poll → STOPPED → destroy` 通过公共 C ABI；
- 非法参数、ABI/header、幂等 shutdown、非法 destroy 和错误线程有确定结果；
- query 不推进，poll 是唯一 Host-pumped 推进入口；
- 当前行为通过 C17 契约测试观察，不依赖私有 C++ seam。

### Epoch B：Minimal GPU Renderer

建立最小 Offscreen Output、FrameTicket、readback 和真实 Vulkan 执行链，先 clear、后 triangle。

完成条件：

- Validation Adapter 确定性推进最小 Frame 状态机；
- Linux + Vulkan Offscreen 生成真实 GPU Evidence Artifact；
- clear 和 triangle 有人工图像、像素断言和设备 provenance；
- HostPumpDriver 与最小 DedicatedDriver spike 复用同一 Frame 契约；
- Win32 Viewer/Swapchain 不阻塞本 Epoch 完成。

### Epoch C：Scene and Workload

建立 Scene Snapshot、资源生命周期和 typed Workload，使用 Cornell Box 验证计划稳定性和资源退休。

完成条件：

- SceneTransaction 产生不可变 SceneVersion；
- Workload Description、Compiled Plan 和 Frame Execution 分离；
- 普通相机、transform 和参数变化不触发计划重编译；
- 在途引用、timeline retirement 和 deferred destruction 通过反例测试。

### Epoch D：Evidence and Performance

完善 diagnostics、timestamp、资源报告、图像回归和稳定帧性能收敛。

完成条件：

- Headless run 输出图像、配置、timings、metrics、resources 和 provenance；
- 稳定帧满足无堆分配、无 I/O、无 pipeline 创建和无 CPU 等待 GPU；
- 固定 benchmark 报告 warm-up、原始数据、p50、p95 和峰值资源。

### Epoch E：First Research Track

以传统 Irradiance Grid 为基线，独立复现并评估 Neural Irradiance，再通过 Irradiance evaluator seam 接入主 Runtime。

完成条件：

- Grid、Neural、Reference 和 Error View 使用相同场景、输入和指标；
- 记录论文核心假设、复现差异、质量和性能结果；
- Neural 路径不向 Host C ABI、Runtime Reactor 或通用 Workload 泄漏论文特有类型；
- 未达到论文收益时允许保留负面结论或删除主线接入。

## 近期纵向 Slices

当前只推进 Slice 1，后续顺序为：

1. **Runtime 生命周期**：实现五个 Host C ABI 生命周期入口；
2. **生命周期反例**：逐个覆盖参数、ABI、destroy、shutdown 和线程错误；
3. **Validation Frame**：最小 Offscreen Output、FrameRequest、FrameTicket 和固定 clear；
4. **Headless 图像产物**：readback、map/unmap、PPM/PNG 和像素断言；
5. **Vulkan Offscreen Clear**：真实 device、queue、image、command、同步和 readback；
6. **Vulkan Triangle**：在 clear 契约稳定后加入 shader、pipeline 和 draw。

Scene、Asset Package、通用 Workload Compiler、Viewer、Swapchain 和研究 Feature 不提前进入上述六个 slice。

## 当前 Slice

当前工作是 `.scratch/rendering-engine-foundation/issues/03-validation-runtime-lifecycle.md`。核心实现由项目实现者完成；Agent 默认负责原理追问、契约测试、分级提示、review 和失败分析，除非收到明确的直接实现请求。
