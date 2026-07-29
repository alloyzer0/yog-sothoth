# Yog-Sothoth 前沿渲染引擎建设大纲

Status: draft

## 1. 愿景

Yog-Sothoth 是一个 GPU-first、可嵌入、可验证的前沿实时渲染内核。它把论文或实验算法编译成受控的 GPU 工作负载，并向游戏、编辑器、仿真、批处理和研究脚本提供稳定的宿主接口。

项目不以复制完整商业游戏引擎为目标。长期价值来自三件事：

1. 前沿算法能够快速接入而不破坏资源、同步和生命周期规则；
2. 同一算法能够在交互、Headless、回归测试和外部宿主中复用；
3. 画质、性能、资源占用和兼容性结论都有可重复的证据。

## 2. 核心产品面

```text
Host SDK / Stable C ABI
  Runtime · SceneTransaction · View · Output · FrameTicket
                           │
Feature Contract / Workload IR
  typed pass · resource contract · history · quality policy
                           │
GPU & Accelerated Runtime
  Vulkan · Compute/RT · external workload · residency · presentation
```

三层必须保持独立：

- Host SDK 不暴露 Vulkan 类型或 Workload Builder；
- Feature 声明资源和执行意图，但不拥有全局队列、交换链或 descriptor heap；
- GPU Runtime 执行编译计划，不理解具体论文或产品功能。

## 3. 设计原则

- **深模块**：对外接口小，资源状态、调度、缓存和诊断隐藏在实现内。
- **先声明、后编译、再执行**：Graph Description、Compiled Plan、Frame Execution 是不同对象。
- **离线优先**：shader、pipeline、材质、几何和模型进入版本化 Render Asset Package。
- **数据驻留优先**：避免帧内 CPU/GPU 往返；场景使用 snapshot、stable handle、SoA 和 dirty range。
- **能力驱动**：Feature 声明需求和 fallback，由结构化 capability table 选择 Adapter 与质量路径。
- **可观测性内建**：图结构、资源寿命、barrier、queue、timestamp、驻留和 fallback 原因必须可查询。
- **逃生口显式负债**：External/Unsafe pass 可用，但必须在诊断中标记其同步和优化限制。

## 4. 建设阶段

### Phase 1：可嵌入渲染内核纵切

建立稳定 C ABI、SceneTransaction、Viewer/Headless Host Adapter、Swapchain/Readback Output、Validation/Vulkan Adapter、typed Workload IR、离线包和基础观测性。以 Direct、Irradiance Grid、Neural Irradiance 和 Error View 贯穿全链路。

交付结果不是商用品质 GI，而是一条可重复、可测量、可被外部宿主调用的架构 tracer bullet。

### Phase 2：高性能调度与场景数据

- graphics/compute/transfer 多队列计划；
- transient alias、history 和跨帧缓存；
- bindless 资源表；
- GPU-driven culling 与 indirect draw；
- Scene Snapshot 分页、驻留预算和 GPU feedback；
- Job System、后台资源解码和 pipeline 预热。

### Phase 3：混合全局光照实验平台

- 统一 Scene Query 和 Surface Cache；
- screen-space、probe/grid、ray tracing 和 neural evaluator 组合；
- 时空复用、质量策略和统一 GPU 预算；
- Reference Path Tracing 与自动画质回归。

### Phase 4：前沿场景表示与加速工作负载

- Gaussian Splatting；
- meshlet/分页几何；
- Neural Radiance Cache 与神经材质；
- CUDA/ONNX/TensorRT 等外部加速 Adapter；
- denoise、upscale 和视频队列。

### Phase 5：生态与分布式集成

- 版本化 Feature SDK；
- Python 实验与参数扫描；
- 外部纹理和跨进程共享；
- 远程/多 GPU Render Worker；
- 编辑器和非破坏式实验 overlay。

## 5. 长期模块地图

| 深模块 | 外部接口承担的概念 | 隐藏的主要复杂性 |
|---|---|---|
| Runtime | 创建、能力、场景提交、渲染、查询 | 线程、帧并行、设备恢复、对象生命周期 |
| Scene Database | transaction、snapshot version | SoA、dirty range、上传、分页、驻留 |
| Workload Compiler | compile(description, policy) | DAG、裁剪、barrier、queue、alias、history |
| Asset Runtime | load(package), resolve(asset) | 版本、fallback、缓存、后台加载、热替换 |
| Accelerated Workload Runtime | compile/dispatch workload | Compute/RT/CUDA/ONNX、互操作和数据驻留 |
| Output | acquire/present/readback | swapchain、外部图像、编码、同步、格式转换 |
| Diagnostics | capture/query/export | timestamp、图、资源寿命、错误和回归证据 |

只在出现至少两个真实 Adapter 时建立 seam。首阶段真实 seam 包括：

- Host：Viewer 与 Headless；
- Output：Swapchain 与 Offscreen Readback；
- GPU：Vulkan 与 Validation；
- Irradiance：Grid 与 Neural。

## 6. 需要 ADR 锁定的决策

1. Host/Output C ABI、线程模型和对象所有权；
2. Feature Contract、Workload IR 与 Unsafe escape hatch；
3. Render Asset Package、版本和 capability fallback；
4. Scene Snapshot、history/cache 与 residency 生命周期；
5. Shader 语言、反射和离线编译工具链；
6. GPU 性能预算的硬件基线与回归规则。

## 7. 明确避免

- 把 Vulkan wrapper 当作完整 Runtime；
- 公共 SDK 暴露 RenderGraphBuilder、barrier 或原生队列；
- 一个巨型 Renderer 接口堆积全部算法入口；
- 每个 Feature 自带 descriptor、同步、模型加载和指标体系；
- 稳定帧中编译 shader、创建 pipeline、访问文件或等待 GPU；
- 只有截图，没有 reference、指标、编译计划和硬件证据。
