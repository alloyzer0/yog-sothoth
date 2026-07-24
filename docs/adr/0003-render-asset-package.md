# ADR-0003：统一 Render Asset Package

Status: accepted for Phase 1

## Context

运行时即时解析 shader、模型、材质和训练产物会引入热路径编译、工具链差异和不可重复结果。不同 Feature 若自定义包格式，也会复制版本、能力与缓存逻辑。

## Decision

- shader、pipeline、mesh、material、Irradiance Grid 和 Neural MLP 权重进入统一 Render Asset Package；
- 包包含 format version、content hash、capability requirements、fallback、反射和 source/toolchain provenance；
- Slang/HLSL 在离线阶段编译为 SPIR-V；Runtime 不编译生产 shader；
- Asset Runtime 只加载验证通过的包，并返回 stable asset handle；
- 不兼容、损坏或能力不足时结构化拒绝，不能静默产生未定义画面；
- Python/PyTorch、Blender、Mitsuba 等属于上游工具，不进入 Runtime 依赖。

## Consequences

- 需要独立 Asset Compiler 和可重复的 package manifest；
- pipeline cache 与 package content hash 可以建立稳定关联；
- 后续几何分页、CUDA/ONNX 模型和视频工作负载可扩展 package，但必须版本化；
- 热重载是重新加载新包版本，不原地修改在途帧使用的资产。

## Rejected

- 每个 Feature 自带资源目录和加载器：重复兼容与缓存逻辑；
- 运行时编译全部 shader/model：破坏稳定帧性能和可重复性；
- 直接加载训练 checkpoint：把训练框架泄漏到 Runtime。

