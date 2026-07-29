# ADR-0004：Scene Snapshot 与资源生命周期

Status: accepted for Phase 1

## Context

高性能渲染不能每帧遍历 authoring 对象并重建 draw call。Scene 更新、跨帧缓存、外部输出和在途 GPU 工作还要求明确的所有权与退休规则。

## Decision

- Host 通过 SceneTransaction 提交增量；commit 生成不可变 SceneVersion；
- commit 仅在返回成功时消费 SceneTransaction；失败必须保持 transaction 内容不变且不产生 SceneVersion；
- Render Scene 使用 stable handle、SoA 和 dirty range；
- 在途帧保留其 SceneVersion，后续 commit 不修改旧版本可见状态；
- GPU 资源分为 transient、persistent、history、external、readback；
- transient 句柄不能逃逸逻辑帧；history 由 Runtime 轮换；external 使用显式 acquire/release 契约；
- 资源删除采用 timeline retirement 和 deferred destruction；
- SceneVersion/FrameTicket 的 Host 引用允许任意线程 release；release 进入并发 inbox，Runtime Reactor 统一处理内部引用和实际退休；
- Offscreen readback 仅绑定 FrameTicket；active lease 保留 ticket 内部 readback 资源并阻止 ticket retirement；
- Output destroy 后旧 revision 由在途 FrameTicket 内部保留，Host handle 与报告/readback 生命周期解耦；
- Transaction write、FeatureProfile 和 Frame 会对引用的 AssetPackage 取得内部引用；Host package release 不破坏这些对象；
- FeatureProfile update 创建新 revision，render 捕获 revision，destroy 不破坏已接受 Frame；
- Phase 1 全量驻留，但 package id、content hash、size 和 residency class 必须存在，以便 Phase 2 引入分页和预算。

## Consequences

- scene commit 需要合并 transaction 并产生 GPU dirty update；
- commit 实现需要在移动 transaction 命令缓冲前建立原子接受点；
- FrameTicket、SceneVersion 与资源退休形成统一生命周期；
- Feature 不能缓存原生 GPU handle 或自行释放 history；
- Validation Adapter 必须检测 use-after-free、非法 transient 逃逸和 external 契约错误。
- 契约测试必须覆盖并发 double-release、release/query 竞态以及 shutdown drain Release Inbox。
- 契约测试必须覆盖 map/unmap 线程亲和、mapped ticket release、shutdown 与 active lease。
- handle identity 包含 Runtime instance/type/slot/generation；ABI 操作通过 operation pin 线性化，终态发布对 query/map/report 建立可见性；
- STOPPED 允许未 release 的纯 Host handle，但 destroy 要求全部 handle/ABI 调用清空；device lost 使用失败清理而非正常 timeline retirement；
- shutdown cutoff 后 release 使用同步 CPU-ref 路径，STOPPED 发布通过 release gate 关闭最终竞态；GPU-backed handle 在 STOPPED 前 detach 为 CPU-only tombstone；

## Rejected

- 原地修改全局 Scene：在途帧会观察不一致状态；
- 纯引用计数即时销毁：无法表达 GPU 尚未完成的引用；
- 把 history 当 persistent texture：隐藏帧间依赖并削弱图验证；
- Phase 1 预先实现伪分页系统：增加复杂度但没有真实使用者。
