# 前沿渲染引擎架构对比调研（2026）

> 调研快照：2026-07-25。本文优先采用官方文档、官方源码、规范与厂商研究资料；“趋势”和对 Yog-Sothoth 的结论属于基于这些一手材料的综合判断，不代表各项目官方立场。

## 结论先行

Yog-Sothoth 不应照搬任何一个现成引擎。更可靠的组合是：

- **外部嵌入形态**参考 Filament 与 bgfx：小型、稳定、opaque-handle 的 C ABI，不泄漏 Vulkan 类型；
- **内部工作负载编译**参考 Unreal RDG、Unity RenderGraph 与 Granite：typed pass、声明式访问、自动同步、裁剪、alias、跨队列计划和显式 history；
- **研究功能组织**参考 Falcor：Feature 反射、图描述与编译计划分离、粗粒度 Python 实验入口；
- **GPU 数据与资产**参考 Nanite：离线编译、分页数据、驻留管理、GPU feedback，而非传统 Mesh/LOD 列表；
- **混合算法编排**参考 Lumen：多场景表示、多追踪 backend、缓存与质量预算；神经网络只是 evaluator，不是整个 GI 架构；
- **宿主与输出集成**参考 RTX Remix/bgfx：Host Adapter、非破坏 overlay、外部纹理、离屏、共享纹理、编码流和 readback 都是一等能力；
- **低层跨 API 与后台系统**参考 Diligent/Wicked：结构化 capability、自动/显式状态并存、SoA Render Scene、Job System；
- 将原先的 `Neural Execution` 泛化成 **Accelerated Workload Runtime**，同时容纳 compute、ray tracing、CUDA/ONNX、splatting、降噪和视频队列。

这会把此前的“论文 Demo 宿主”修正为三层稳定产品边界：

```text
Host SDK / C ABI
  Runtime · Scene Transaction · View · Output · Experiment
                         │
Feature ABI / Workload IR
  typed pass · resource contract · history · quality/capability policy
                         │
GPU & Accelerated Runtime
  Vulkan · Compute/RT · CUDA/ONNX · memory/residency · presentation/encode
```

## 对比矩阵

| 系统 | 最值得借鉴 | 明确不要照搬 |
|---|---|---|
| Unreal RDG | 声明式资源访问、编译期裁剪/同步/transient、观测性 | 与 UE 宏、shader parameters、RHI 深耦合的公共接口 |
| Nanite | 离线 cluster 编译、分页驻留、GPU feedback/决策 | 把 meshlet culling 单独包装成“虚拟几何” |
| Lumen | 多表示、多 backend、跨帧 cache、统一预算 | 假设一个 RT backend 或一个神经模型覆盖全部内容 |
| Unity SRP/RenderGraph | 配置与执行分离、typed pass、严格 handle epoch、图查看器 | Compatibility/Unsafe 成为默认扩展接口 |
| Filament | 深而小的嵌入 API、Engine ownership、离线材质包、DriverApi | 固定 PBR pipeline 作为研究平台的功能边界 |
| Diligent | 可裁剪 RHI、capability、自动/显式状态、离线 PSO | 把 RHI 当成 workload compiler |
| Falcor | Feature 反射、DAG、插件、Python 研究流程 | 字符串资源名/Dictionary/C++ ABI 直接成为长期 SDK |
| RTX Remix | Host Adapter、捕获/替换/重渲染、authoring/runtime overlay | 围绕 D3D8/9 拦截或单一厂商生态设计核心 |
| Wicked | ECS/SoA、Job System、GPU scene 与算法纵向集成 | 巨型 Renderer facade 和紧耦合全家桶 |
| Granite | 跨队列计划、alias/subpass/history、Vulkan 深度 | 将个人项目直接作为稳定产品依赖 |
| bgfx | C99 ABI、opaque handle、Encoder、外部资源、Null backend | 期待提交层自动解决场景和跨 pass 生命周期 |
| WebGPU | 可移植安全边界、明确 bind group/pipeline layout、浏览器/远程入口 | 以其最小公共能力限制 native 高端后端 |

## 重点案例

### Unreal：RDG、Nanite、Lumen 是三种不同层面的答案

RDG 是“收集声明、延迟编译、再执行”，而非简单 pass 列表。资源读写导出依赖，编译器负责不可达 pass 裁剪、资源寿命、transient 复用、状态转换和 async-compute 范围；external/extracted resource 显式处理跨图持久化。其 escape hatch 很必要，但 `NeverCull` 和图外访问会削弱验证与优化。[RDG 官方指南](https://dev.epicgames.com/documentation/en-us/unreal-engine/render-dependency-graph-in-unreal-engine)、[官方源码入口（需 Epic 账号）](https://github.com/EpicGames/UnrealEngine/tree/release/Engine/Source/Runtime/RenderCore/Public/RenderGraph)

Nanite 的关键是把资产构建、层级 cluster、屏幕误差、GPU 可见性、分页流送和软/硬件栅格联合设计。它说明 Render Scene 必须预留“编译后的分页 GPU 数据集 + residency manager + feedback”边界；但首阶段只应建立 package/residency seam，不应承诺复制 Nanite。[Nanite 官方技术文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/nanite-virtualized-geometry-in-unreal-engine)、[Epic SIGGRAPH 2021 课程](https://advances.realtimerendering.com/s2021/index.html)

Lumen 则证明实时 GI 是多表示混合系统：screen traces、软件 RT 的距离场、硬件 RT、Surface Cache/Cards 和时空复用共同工作。Neural Irradiance 应接入共享的 Scene Query、Surface Cache/history 和 quality policy，而非自建一条孤立 pipeline。[Lumen 技术细节](https://dev.epicgames.com/documentation/en-us/unreal-engine/lumen-technical-details-in-unreal-engine)、[Lumen 性能指南](https://dev.epicgames.com/documentation/en-us/unreal-engine/lumen-performance-guide-for-unreal-engine)

### Unity 6：RenderGraph 已从可选优化变成主路径

SRP 将持久的 `RenderPipelineAsset` 与逐帧 `RenderPipeline` 分离；URP 以受控的 Feature/Pass seam 扩展。[SRP 概览](https://docs.unity3d.com/6000.0/Documentation/Manual/srp-introduction.html)、[Unity Graphics 官方源码](https://github.com/Unity-Technologies/Graphics)

Unity 6 URP 通过 `RecordRenderGraph` 记录工作，Raster/Compute/Unsafe pass 明确执行域，handle 仅在图的有效期内成立；编译器管理生命周期、裁剪、同步，并可合并兼容 raster pass。Compatibility/Unsafe 是迁移口而不是理想主路径。Yog-Sothoth 应采用 typed pass、严格 epoch handle、typed frame context，并在首阶段同时交付图查看器。[RenderGraph 总览](https://docs.unity3d.com/6000.0/Documentation/Manual/urp/render-graph.html)、[优化说明](https://docs.unity3d.com/6000.0/Documentation/Manual/urp/render-graph-introduction.html)、[写 Render Pass](https://docs.unity3d.com/6000.0/Documentation/Manual/urp/render-graph-write-render-pass.html)、[RenderGraph Viewer](https://docs.unity3d.com/6000.0/Documentation/Manual/urp/render-graph-view.html)、[官方实现](https://github.com/Unity-Technologies/Graphics/tree/master/Packages/com.unity.render-pipelines.core/Runtime/RenderGraph)

### Filament：最接近理想的嵌入式外壳

Filament 对外以 Engine 管理资源，应用只组合 Scene/View/Camera/Renderer/SwapChain；内部用 component manager、DriverApi/CommandStream 和 FrameGraph 隐藏平台复杂性。材质通过 `matc` 离线编译为 package，运行时只实例化和设参。适合借鉴其 ownership、SoA、package 与 backend seam，但其固定实时 PBR 产品目标不足以承载任意研究图。[架构与渲染说明](https://google.github.io/filament/Filament.md.html)、[Engine API](https://github.com/google/filament/blob/main/filament/include/filament/Engine.h)、[Renderer API](https://github.com/google/filament/blob/main/filament/include/filament/Renderer.h)、[FrameGraph](https://github.com/google/filament/tree/main/filament/src/fg)、[DriverApi](https://github.com/google/filament/blob/main/filament/backend/include/private/backend/DriverApi.h)

### Falcor 与 Granite：研究工作流和执行计划应分别借鉴

Falcor 的 `RenderPass::reflect()`、DAG、Graph compiler/executable 分离、共享库插件和 Python 图配置非常适合快速论文实验。Yog-Sothoth 可采用 `declare/prepare/record` 三阶段 Feature，但对外仍需版本化 C ABI，不能让字符串资源名、动态 Dictionary 或 C++ ABI 成为产品边界。[Falcor](https://github.com/NVIDIAGameWorks/Falcor)、[RenderGraph API](https://github.com/NVIDIAGameWorks/Falcor/blob/master/Source/Falcor/RenderGraph/RenderGraph.h)、[Compiler](https://github.com/NVIDIAGameWorks/Falcor/blob/master/Source/Falcor/RenderGraph/RenderGraphCompiler.cpp)、[RenderPass 反射](https://github.com/NVIDIAGameWorks/Falcor/blob/master/Source/Falcor/RenderGraph/RenderPass.h)、[Python 入口](https://github.com/NVIDIAGameWorks/Falcor/blob/master/docs/falcor-in-python.md)

Granite 的图编译更接近高性能执行内核：自动 layout/load-store、跨队列 semaphore、同队列 event、尽早 signal/尽晚 wait、alias、subpass、transient attachment、history、条件 pass 和 pass 重排。它提示 Workload Compiler 必须联合决定顺序、queue、同步点、alias 与 tile/immediate GPU 策略，而不是只生成 barrier；但该项目明确不是稳定 SDK。[Granite](https://github.com/Themaister/Granite)、[Render Graph 接口](https://github.com/Themaister/Granite/blob/master/renderer/render_graph.hpp)、[编译/执行实现](https://github.com/Themaister/Granite/blob/master/renderer/render_graph.cpp)、[项目总览](https://github.com/Themaister/Granite/blob/master/OVERVIEW.md)

### Diligent、Wicked 与 bgfx：分别回答 RHI、纵向数据和 ABI

Diligent 的价值在可裁剪跨 API RHI：统一 shader/resource binding、结构化 capability、自动或显式资源状态，以及离线 Render State Packager。由此应保留专家显式模式，但让默认 Feature 使用受控自动模式；RHI 与图编译器必须分层。[Diligent Engine](https://github.com/DiligentGraphics/DiligentEngine)、[DiligentCore](https://github.com/DiligentGraphics/DiligentCore)、[资源状态管理](https://diligentgraphics.com/2018/12/09/resource-state-management/)、[Render State Packager](https://github.com/DiligentGraphics/DiligentTools/tree/master/RenderStatePackager)

Wicked 展示 ECS/SoA Scene、Job System、Graphics Device、Shader Interop 和 GPU-driven 算法如何纵向协作。应借其 render-world 和后台任务设计，但避免复制大型 `Renderer` facade。[Wicked Engine](https://github.com/turanszkij/WickedEngine)、[Scene/ECS](https://github.com/turanszkij/WickedEngine/blob/master/WickedEngine/wiScene.h)、[Job System](https://github.com/turanszkij/WickedEngine/blob/master/WickedEngine/wiJobSystem.h)、[Graphics Device](https://github.com/turanszkij/WickedEngine/blob/master/WickedEngine/wiGraphicsDevice.h)

bgfx 则证明稳定 C99 API、opaque handle、自定义 allocator/callback、capability table、多线程 Encoder、外部 framebuffer 和 Noop backend 的长期价值；也反向证明 RHI/提交层不会替应用解决 Render Graph 和场景生命周期。[bgfx Overview](https://bkaradzic.github.io/bgfx/overview.html)、[API Reference](https://bkaradzic.github.io/bgfx/bgfx.html)、[公共头文件](https://github.com/bkaradzic/bgfx/blob/master/include/bgfx/bgfx.h)

### RTX Remix：集成不只是“传一个窗口句柄”

RTX Remix 把旧应用捕获、非破坏资产/材质替换和路径追踪 runtime 分离，并把 authoring 与 runtime 分成不同产品。这要求 Yog-Sothoth 增加 `Host Adapter` 与版本化 overlay：原生宿主、外部纹理、API capture、IPC 都先归一化为 Render Scene Snapshot；兼容性协商和诊断应是接口的一部分。[RTX Remix combined repo](https://github.com/NVIDIAGameWorks/rtx-remix)、[dxvk-remix Runtime](https://github.com/NVIDIAGameWorks/dxvk-remix)、[toolkit-remix](https://github.com/NVIDIAGameWorks/toolkit-remix)、[Runtime Guide](https://github.com/NVIDIAGameWorks/rtx-remix/wiki/runtime-user-guide)

## Vulkan、WebGPU 与神经/加速运行时

Vulkan 的现代实现不应再围绕“手写每条 barrier + 每材质 descriptor set”构建。Dynamic Rendering、Synchronization2、timeline semaphore、descriptor indexing/buffer、graphics pipeline library/shader object、mesh/ray tracing 与 Vulkan Profiles 共同推动“能力协商 + 编译计划 + bindless/设备地址”的 GPU runtime。具体扩展必须通过 capability table 选择，不能把可选扩展写成跨设备 ABI 保证。[Vulkan Guide](https://docs.vulkan.org/guide/latest/)、[Vulkan Profiles](https://github.com/KhronosGroup/Vulkan-Profiles)、[Vulkan Samples](https://github.com/KhronosGroup/Vulkan-Samples)、[Vulkan 规范](https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html)

WebGPU 的 bind group、pipeline layout、command encoder/pass encoder 和严格资源使用规则提供了优秀的安全、可移植边界，适合作为浏览器、远程可视化或教学 backend；但其公共能力与安全模型不应反向限制 native Vulkan 的 RT、稀疏驻留或厂商推理扩展。建议让 WebGPU Adapter 实现一个 capability 子集，而不是把 Workload IR 等同于 WebGPU API。[WebGPU 规范](https://www.w3.org/TR/webgpu/)、[Dawn](https://dawn.googlesource.com/dawn)、[wgpu](https://github.com/gfx-rs/wgpu)

神经推理的真正边界是数据驻留与调度，而不是模型加载函数。ONNX Runtime 的 Execution Provider/IO Binding、CUDA external memory/semaphore、TensorRT 或融合小 MLP 都说明：输入输出要以 device tensor/external resource 表达，避免每帧 CPU 往返；模型编译产物要固定输入语义、精度、shape/batch、设备需求与版本。[ONNX Runtime Execution Providers](https://onnxruntime.ai/docs/execution-providers/)、[IO Binding](https://onnxruntime.ai/docs/performance/tune-performance/iobinding.html)、[CUDA 外部资源互操作](https://docs.nvidia.com/cuda/cuda-c-programming-guide/index.html#external-resource-interoperability)、[tiny-cuda-nn](https://github.com/NVlabs/tiny-cuda-nn)、[Slang](https://github.com/shader-slang/slang)

因此建议接口命名为 `AcceleratedWorkload`，backend 包括 Vulkan Compute、RT pipeline、CUDA、ONNX/TensorRT 和视频队列；`NeuralModel` 只是其中一种 package，不是上层 Feature 的唯一形状。

## 2024—2026 趋势判断

1. **Render Graph 成为默认编译层。** 新重点是 typed pass、原生 pass 合并、跨队列计划、验证和可视化，而非仅拓扑排序。
2. **GPU-driven 进入数据虚拟化阶段。** GPU 决策、分页驻留、feedback streaming 和 offline cook 必须共同设计。
3. **混合渲染取代单算法 renderer。** screen-space、software/hardware RT、cache、传统 compute 和 neural evaluator 在 capability/budget policy 下组合。
4. **离线 package 是稳定性和性能基础。** Shader、pipeline、材质、几何页和模型都进入统一的版本化 Render Asset Package，而非运行时解析 authoring 数据。
5. **互操作与输出成为核心。** Output 不只等于 swapchain，还包括 shared image、编码流、readback、headless 和跨进程 worker。
6. **escape hatch 可用但必须可观测。** Unsafe/native pass 要在图查看器中标记优化债务、外部状态和无法 alias/cull 的原因。
7. **观测性属于编译器。** GPU timestamp、资源寿命/alias、驻留、pass 裁剪原因、queue overlap 和模型/传输成本应在首阶段可见。

## 对 Yog-Sothoth 的架构修正

### 现在应锁定

1. `Host Adapter` 与 `Output` 独立：窗口、headless、external image、共享纹理、编码和 readback 并列。
2. `Feature ABI != GPU ABI`：Feature 声明工作负载；RHI 是内部/专家 seam；插件不能拥有全局 queue、descriptor heap 或 swapchain。
3. Graph description、compiled plan、per-frame execution 三对象分离；内容变化不触发图重编译。
4. Render Scene 采用 snapshot + transaction + stable handle + SoA + dirty range，并预留分页资产、GPU feedback 和 residency。
5. history/cache/external resource 是显式生命周期类别，不伪装成 transient texture。
6. 建立统一 Offline Package Compiler；shader/pipeline/material/geometry/neural model 共用 manifest、版本、capability 与 fallback 机制。
7. 建立 Null/Validation Runtime，在无 GPU CI 中验证图、资源寿命、ABI、package 和 Feature manifest。
8. Python 只做实验配置、批处理和指标读取，不进入逐 draw/逐像素热路径。

### 首阶段应实现

- Vulkan backend + Null/Validation backend；
- 小型 C ABI：Runtime、SceneTransaction、View、Output、FrameTicket；
- typed Workload IR v1：Raster、Compute、Copy、External/Unsafe（显式标债）；
- 单队列正确性后，再增加 async compute；timeline retirement 和 deferred destruction 从一开始存在；
- versioned shader/pipeline/neural package；
- Viewer 与 headless CLI 两个 host adapter；swapchain、offscreen/readback 两类 output；
- Direct、Irradiance Grid、Neural Irradiance、Reference/Error 四类 Feature；
- 图查看、GPU timestamp、资源生命周期、模型/传输开销报告。

### 应延后

- 完整 Nanite 类虚拟几何、透明/变形 fallback 全套；
- 任意第三方 native GPU 插件 ABI；
- D3D12/WebGPU 全功能对等；
- 跨进程 capture/interception；
- 通用训练框架和动态场景 GI。

### 应避免

- 公共 SDK 暴露 Vulkan handle、barrier 或 RenderGraphBuilder；
- 每个 Feature 自带 ONNX/CUDA/descriptor/同步体系；
- 用一个巨型 `Renderer` facade 堆积所有算法入口；
- 运行时编译所有 shader/model/asset，或在稳定帧中创建 pipeline、分配堆、等待 CPU；
- 把“画面能显示”当成完成，而没有 reference、指标、图计划和硬件能力证据。

## 建议下一步决策

下一轮设计不应先画更多类图，而应形成四份 ADR：

1. **Host/Output ABI 与线程模型**；
2. **Feature Contract 与 Workload IR 的能力边界**；
3. **Render Asset Package、版本与 fallback**；
4. **Render Scene Snapshot、history/cache 与 residency 生命周期**。

这四项决定后，Neural Irradiance Volume 才能作为第一条 tracer bullet 验证整套架构，而不是反过来塑造整个引擎。
