# ADR-0002：Feature Contract 与 Workload IR

Status: accepted for Phase 1

## Context

前沿算法需要快速接入，但如果 Feature 直接操作设备、barrier 和全局资源，各算法会各自建立同步与生命周期体系，Workload Compiler 无法验证或优化。

## Decision

- Feature 使用 `declare → prepare → record` 生命周期；
- `declare` 生成 typed Graph Description，描述 pass、资源、能力和输出；
- Workload Compiler 把 Graph Description 编译为不可变 Compiled Plan；
- Frame Execution 只更新动态参数并执行已有计划；
- IR v1 支持 Raster、Compute、Copy、External/Unsafe pass；
- 资源显式分类为 transient、persistent、history、external、readback；
- Feature 不拥有全局 queue、swapchain、descriptor heap 或资源销毁；
- External/Unsafe 是受诊断的 escape hatch，必须声明副作用和状态；
- Phase 1 以单 graphics queue 保证正确性，但 IR 不固化队列映射。

## Consequences

- 普通 SceneVersion、相机和参数变化不会触发图重编译；
- Workload Compiler 可以统一做验证、裁剪、barrier、寿命和 alias 计划；
- Feature interface 保持较小，复杂调度集中获得 locality；
- 首阶段必须交付图导出和故障注入测试，否则声明契约无法验证。

## Rejected

- 公开 RenderGraphBuilder 给 Host：接口过大且泄漏内部执行模型；
- 每个 Feature 直接记录原生 Vulkan：无法跨 Adapter 验证；
- 把每个编译步骤公开为独立模块：形成浅接口并增加调用者知识负担。

