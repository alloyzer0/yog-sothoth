# Phase 1 规格：可嵌入渲染内核与 Neural Irradiance 纵切

Status: draft

架构约束由 [架构视图 v1](../../docs/architecture/README.md) 和 ADR-0001～0004 固定。实现若需要违反其中的不变量，必须先修订 ADR。

## 1. 目的

Phase 1 验证 Yog-Sothoth 的产品边界和高性能执行骨架是否成立。完成后，同一份场景、Feature 配置和资产包应能：

1. 被交互式 Viewer 和 Headless CLI 驱动；
2. 通过稳定 C ABI 使用，不向宿主泄漏 Vulkan；
3. 在 Null/Validation Adapter 上验证契约，在 Vulkan Adapter 上生成图像；
4. 运行 Direct、Irradiance Grid 和 Neural Irradiance 三条路径；
5. 输出图结构、资源寿命、GPU 时间、画质误差和兼容性证据。

## 2. 技术基线

- 语言：C++20；公共接口为 C17 ABI；
- 构建：CMake + CMake Presets；
- 首个平台：Windows x64；保持平台层可替换，但 Phase 1 不要求 Linux 验收；
- GPU 后端：Vulkan 1.3；使用 dynamic rendering、Synchronization2 和 timeline semaphore；
- Shader：Slang/HLSL 离线编译为 SPIR-V；运行时不编译生产 shader；
- 测试：单元/契约测试加 Headless 图像回归；
- 实验训练：允许 Python/PyTorch 离线训练，但运行时不得依赖 Python。

若工具链实测暴露阻塞，必须用 ADR 修改，而不是在实现中静默更换。

## 3. 范围

### 必须实现

- Runtime、SceneTransaction、View、Output、FrameTicket 的 C ABI；
- Viewer 与 Headless CLI 两个 Host Adapter；
- Swapchain 与 Offscreen Readback 两个 Output Adapter；
- Null/Validation 与 Vulkan 两个 GPU Adapter；
- typed Workload IR v1；
- Graph Description、Compiled Plan、Frame Execution 分离；
- Render Asset Package v1；
- 最小 Scene Snapshot；
- Direct、Irradiance Grid、Neural Irradiance、Reference/Error Feature；
- 图导出、资源寿命、CPU/GPU timestamp 和错误诊断；
- Cornell Box 固定验证场景。

### 明确不实现

- 通用编辑器、动画、物理和脚本系统；
- D3D12/WebGPU Adapter；
- 完整路径追踪器；参考数据允许由外部离线工具生成；
- 任意第三方 native GPU 插件 ABI；
- 多 GPU、IPC、远程渲染和视频编码；
- 分页虚拟几何、GPU-driven culling 和完整 bindless 材质系统；
- 动态几何、动态 GI 和通用跨场景神经模型；
- CUDA/ONNX Runtime 集成。

这些能力只在接口中避免被阻断，不做占位实现。

## 4. 外部接口

公共 ABI 使用 opaque handle、显式版本和结构体大小字段。关键位置必须有中文注释说明所有权、线程和生命周期约束。

```c
typedef struct ys_runtime_t ys_runtime_t;
typedef uint64_t ys_scene_version;
typedef uint64_t ys_view_handle;
typedef uint64_t ys_output_handle;
typedef uint64_t ys_frame_ticket;

typedef struct ys_struct_header {
    uint32_t struct_size;
    uint32_t abi_version;
} ys_struct_header;

ys_result ys_runtime_create(
    const ys_runtime_desc* desc,
    ys_runtime_t** out_runtime);

ys_result ys_scene_commit(
    ys_runtime_t* runtime,
    const ys_scene_transaction* transaction,
    ys_scene_version* out_version);

ys_result ys_render(
    ys_runtime_t* runtime,
    const ys_frame_request* request,
    ys_frame_ticket* out_ticket);

ys_result ys_frame_query(
    ys_runtime_t* runtime,
    ys_frame_ticket ticket,
    ys_frame_status* out_status);
```

### 接口约束

- `ys_render` 非阻塞；完成状态通过 `FrameTicket` 查询；
- scene commit 生成不可变版本，进行中的帧继续引用旧版本；
- Runtime 拥有内部 GPU 对象，宿主只持 opaque handle；
- 回调不得从未声明线程重入宿主；
- 错误返回结构化 code、message、subsystem 和可选 diagnostics id；
- ABI 结构允许尾部扩展；未知字段按 `struct_size` 忽略。

## 5. Host 与 Output

### Viewer Host Adapter

- 创建窗口、相机和输入；
- 切换 Direct/Grid/Neural/Reference/Error；
- 展示帧时间、Feature 路径和 fallback 原因；
- 不直接调用 Vulkan。

### Headless Host Adapter

- 从配置文件加载场景、资产包和实验参数；
- 固定相机与随机种子；
- 输出 PNG、JSON metrics 和 Workload 图；
- 返回适合 CI 的退出码。

### Output Adapter

- Swapchain Output：窗口显示；
- Offscreen Readback Output：异步复制并在完成后读取；
- Output 描述格式、尺寸、色彩空间和用途；
- Feature 不能直接 present 或 readback。

## 6. Scene Snapshot v1

支持：

- stable entity handle；
- transform、mesh、material、light、camera；
- packed SoA GPU 数据；
- transaction 合并与 dirty range；
- scene version 与 frame 引用；
- 静态 Cornell Box 资产。

每帧不得遍历 authoring 对象生成 draw call。Scene commit 负责把增量编译成 Render Scene 更新。

## 7. Workload IR v1

### Pass 类型

- Raster；
- Compute；
- Copy；
- External/Unsafe，仅用于明确无法表达的工作，必须携带副作用和资源状态声明。

### Resource 类型

- transient；
- imported/external；
- persistent；
- history；
- readback。

### 编译职责

- 校验读写声明、格式、usage 和生命周期；
- 构建依赖 DAG；
- 删除不可达且无副作用的 pass；
- 生成 Vulkan barrier 和 layout transition；
- 规划 transient 资源复用；
- 缓存 Compiled Plan；
- 导出 pass、资源、依赖、barrier 和寿命信息。

Phase 1 先保证单 graphics queue 正确性。Compute pass 可以存在，但 async compute 排期属于 Phase 2；接口不能假设 Compute 永远与 Graphics 同队列。

### 重新编译规则

- Feature 拓扑、Output 格式/尺寸类别或 capability 路径变化：重新编译；
- transform、材质参数、相机和普通场景内容变化：不得重新编译；
- diagnostics 必须记录重新编译原因和耗时。

## 8. GPU Runtime v1

### Vulkan Adapter

- 设备选择和结构化 capability report；
- queue、command pool 和 command buffer 生命周期；
- timeline retirement；
- deferred destruction；
- staging ring；
- pipeline cache 持久化；
- GPU timestamp；
- validation layer 集成；
- 设备不满足要求时输出精确 capability 差异。

### Null/Validation Adapter

它不是返回成功的空壳，必须验证：

- handle 和对象生命周期；
- pass 的未声明读写；
- transient/history/external 资源规则；
- scene version 与 frame ticket；
- package 版本和 capability；
- Feature manifest 和 Workload IR 可编译性。

它不验证 shader 数值结果，但必须让大部分契约测试无需 GPU 运行。

## 9. Render Asset Package v1

统一包至少包含：

- manifest、format version、content hash；
- shader module 和反射；
- pipeline description；
- mesh/material 数据；
- Irradiance Grid 或 Neural MLP 权重；
- capability requirements；
- fallback asset 引用；
- source/toolchain provenance。

运行时只读取包，不解析训练工程或 authoring shader。包不兼容时必须拒绝并说明具体字段，而不是崩溃或产生未定义画面。

## 10. Feature Contract v1

Feature 生命周期分三阶段：

```text
declare  → 声明资源、pass、能力和输出
prepare  → 更新每帧参数和持久状态
record   → 为已编译计划记录命令
```

Feature 可以拥有跨帧逻辑状态，但 GPU 资源分配、同步和销毁归 Runtime。

首阶段 Feature：

1. `DirectLighting`：Lambertian 直接光基线；
2. `IrradianceGrid`：3D Grid + 三线性插值；
3. `NeuralIrradiance`：固定结构小型 MLP 的 Vulkan Compute/Shader 推理；
4. `ReferenceImage`：加载固定相机参考间接光；
5. `ErrorView`：输出误差热图和指标。

Grid 与 Neural 是同一 Irradiance evaluator seam 上的两个真实 Adapter，不各自复制渲染管线。

## 11. Neural Irradiance 实验

### 数据

- 一个固定 Cornell Box；
- 20,000～100,000 条表面样本；
- 每条包含 position、normal、irradiance RGB；
- 固定训练/验证/测试划分与随机种子；
- 参考数据生成工具和版本写入 provenance。

### 模型

- 输入：归一化 position + normal；
- 输出：非负 RGB irradiance；
- 固定小型 MLP，参数包目标不超过 1 MiB；
- FP32 为正确性基线，FP16 为性能路径；
- 训练实现不属于 Runtime；只导入已编译权重。

### 对照

- Direct only；
- 固定常量环境光；
- Irradiance Grid；
- Neural Irradiance；
- Reference。

## 12. 观测性与证据

每次 Headless 实验输出：

```text
artifacts/<run-id>/
├─ config.json
├─ capabilities.json
├─ workload.json
├─ timings.json
├─ metrics.json
├─ direct.png
├─ grid.png
├─ neural.png
├─ reference.png
└─ error.png
```

必须记录：

- CPU frame、graph compile 和 scene commit 时间；
- 每个 pass 的 GPU timestamp；
- 资源峰值与 transient 复用情况；
- pipeline cache hit/miss；
- package、shader、模型和设备标识；
- MSE、PSNR，SSIM 为可选；
- fallback、Unsafe pass 和 validation warning。

## 13. 性能预算

以下是 Phase 1 工程预算，不是跨硬件性能承诺。正式验收必须记录 GPU/驱动/分辨率。

| 指标 | 目标 |
|---|---:|
| 场景不变的稳定帧 CPU 堆分配 | 0 |
| 稳定帧 shader/pipeline/file I/O | 0 |
| `ys_render` CPU 提交开销 | p95 小于 0.5 ms |
| Neural Irradiance 额外 GPU 成本 | 1080p 下 p95 小于 2.0 ms |
| Neural 权重包 | 不超过 1 MiB |
| Workload 重编译 | 普通场景/相机变化时为 0 |
| CPU 阻塞等待 GPU | 稳定帧为 0 |
| Validation error | 0 |

若开发机不满足 GPU 指标，仍可完成正确性里程碑，但不得宣称性能验收通过；必须记录硬件边界和实测结果。

## 14. 验收场景

### A. 嵌入一致性

Viewer 与 Headless 通过同一 C ABI 加载同一包和场景，相同相机下的 Offscreen 输出在允许误差内一致。

### B. Validation 替换

同一组 SceneTransaction 和 Workload 描述在 Null/Validation Adapter 通过；故意制造未声明写入时测试必须失败并指出 pass/resource。

### C. 计划稳定性

连续修改相机和 transform 1,000 帧，Compiled Plan 标识保持不变；切换 Output 格式时产生一次带原因的重编译。

### D. 生命周期

销毁仍被在途帧引用的场景资源不会提前释放；timeline 完成后自动回收，无泄漏和 use-after-free。

### E. 光照纵切

Grid 和 Neural 路径都产生颜色反弹；Neural 在保留测试视角上的误差优于常量环境光基线，并生成完整证据包。

### F. 性能

运行固定 2,000 帧基准，报告 warm-up、p50、p95、峰值显存和硬件信息；满足第 13 节预算才标记性能验收通过。

## 15. 里程碑顺序

1. ABI 与 Validation：Runtime、Scene、Output、FrameTicket 的契约测试；
2. Vulkan Bootstrap：三角形经 typed Workload IR 输出到 Headless；
3. 双 Host/双 Output：Viewer 与 CLI 复用同一 Runtime；
4. Scene Snapshot：Cornell Box、增量 transform、稳定计划；
5. Asset Package：离线 shader/pipeline/mesh 加载和版本验证；
6. Grid Irradiance：建立非神经基线；
7. Neural Irradiance：模型包、FP32/FP16 推理；
8. Diagnostics：图、资源寿命、timestamp、误差和 CI 产物；
9. 性能收敛：删除稳定帧分配/等待，形成 Phase 1 证据报告。

每个里程碑必须形成可运行 tracer bullet，不按“先写完整 RHI、再写完整场景、最后集成”的横向大批量方式推进。

## 16. 完成定义

Phase 1 只有同时满足以下条件才完成：

- 两个 Host、两个 Output、两个 GPU Adapter 均通过契约；
- Cornell Box 的 Grid/Neural/Reference/Error 可由 Viewer 和 Headless 运行；
- 计划稳定性、生命周期和故障注入测试通过；
- 输出完整、可重复的证据包；
- 公共 C ABI 未泄漏 Vulkan 和训练框架类型；
- 性能结果明确区分“达到预算”和“受硬件限制未验收”；
- 已记录 Phase 2 所需 ADR 和未解决风险，不以 TODO 代替接口约束。

## 17. 主要风险

| 风险 | 控制方式 |
|---|---|
| 首阶段基础设施过重 | 每个里程碑保持端到端 tracer bullet；不提前实现 Phase 2 优化 |
| Workload IR 过度抽象 | 只覆盖五个首阶段 Feature；新能力用真实需求扩展 |
| C ABI 限制内部设计 | ABI 只表达宿主操作，不暴露 Feature/GPU 内部对象 |
| Neural 路径绑架架构 | Grid 与 Neural 共用 evaluator seam；Accelerated Workload 保持通用 |
| 外部参考数据不可复现 | 固定工具版本、场景、随机种子和 provenance |
| Vulkan 能力差异 | capability report、明确最低配置和结构化 fallback |
