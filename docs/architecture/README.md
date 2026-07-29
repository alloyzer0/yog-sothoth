# Yog-Sothoth 架构视图 v1

本文档集描述 Runtime 的长期目标系统关系、运行时执行、工作负载和资源生命周期。它们约束模块的 interface、seam、所有权和时序，不作为类图或源码目录设计。

## 视图索引

- [C4 Context 与 Container](c4.md)
- [帧执行时序](runtime-execution.md)
- [GPU 工作负载](workload.md)
- [资源生命周期](resource-lifecycle.md)

## 决策索引

- [ADR-0001：Host/Output C ABI 与线程模型](../adr/0001-host-output-abi.md)
- [ADR-0002：Feature Contract 与 Workload IR](../adr/0002-feature-workload-contract.md)
- [ADR-0003：统一 Render Asset Package](../adr/0003-render-asset-package.md)
- [ADR-0004：Scene Snapshot 与资源生命周期](../adr/0004-scene-resource-lifetime.md)
- [ADR-0005：内部 C++23 语言基线与 C ABI 隔离](../adr/0005-internal-cpp23-language-baseline.md)
- [ADR-0006：Headless-first Vulkan Bootstrap 与证据输出](../adr/0006-headless-first-vulkan-bootstrap.md)

## 架构不变量

1. Host 通过稳定 C ABI 使用 Runtime，不接触 Vulkan 或 Workload Builder。
2. Feature 声明工作负载，不拥有全局 GPU 队列、交换链和 descriptor heap。
3. Graph Description、Compiled Plan、Frame Execution 是不同生命周期的对象。
4. Scene commit 产生不可变 SceneVersion；在途帧可以安全引用旧版本。
5. Output 独立于 Host；窗口不是唯一输出方式。
6. Runtime 只加载离线编译、版本化且可验证的 Render Asset Package。
7. Validation 与 Vulkan 是 GPU seam 上的两个真实 Adapter。
8. 内部 C++23 implementation 不得把语言、标准库或异常要求传播到 C17 Host ABI。

## 与执行路线图的关系

长期能力规格位于 [spec.md](../../.scratch/rendering-engine-foundation/spec.md)，当前执行顺序与阶段完成条件位于 [roadmap.md](../../.scratch/rendering-engine-foundation/roadmap.md)。如果实现需要违反本页不变量，必须先新增或修订 ADR，不能仅修改图或代码。
