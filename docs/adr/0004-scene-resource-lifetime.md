# ADR-0004：Scene Snapshot 与资源生命周期

Status: accepted for Phase 1

## Context

高性能渲染不能每帧遍历 authoring 对象并重建 draw call。Scene 更新、跨帧缓存、外部输出和在途 GPU 工作还要求明确的所有权与退休规则。

## Decision

- Host 通过 SceneTransaction 提交增量；commit 生成不可变 SceneVersion；
- Render Scene 使用 stable handle、SoA 和 dirty range；
- 在途帧保留其 SceneVersion，后续 commit 不修改旧版本可见状态；
- GPU 资源分为 transient、persistent、history、external、readback；
- transient 句柄不能逃逸逻辑帧；history 由 Runtime 轮换；external 使用显式 acquire/release 契约；
- 资源删除采用 timeline retirement 和 deferred destruction；
- Phase 1 全量驻留，但 package id、content hash、size 和 residency class 必须存在，以便 Phase 2 引入分页和预算。

## Consequences

- scene commit 需要合并 transaction 并产生 GPU dirty update；
- FrameTicket、SceneVersion 与资源退休形成统一生命周期；
- Feature 不能缓存原生 GPU handle 或自行释放 history；
- Null/Validation Adapter 必须检测 use-after-free、非法 transient 逃逸和 external 契约错误。

## Rejected

- 原地修改全局 Scene：在途帧会观察不一致状态；
- 纯引用计数即时销毁：无法表达 GPU 尚未完成的引用；
- 把 history 当 persistent texture：隐藏帧间依赖并削弱图验证；
- Phase 1 预先实现伪分页系统：增加复杂度但没有真实使用者。

