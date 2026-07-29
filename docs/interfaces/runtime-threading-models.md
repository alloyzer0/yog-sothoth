# Runtime 线程模型候选

Status: accepted for Phase 1

## 共同不变量

- Runtime 独占 GPU device、queue、同步、descriptor 和资源生命周期；
- Host CPU submit queue 永远不等于 Vulkan queue；
- `ys_render` 非阻塞；
- FrameTicket 终态和资源退休语义不随线程模型变化；
- 线程模型在 Runtime 创建后不可切换；
- query 不隐式推进工作；
- 不从任意内部线程调用 Host callback。
- SceneVersion/FrameTicket release 可从任意 Host 线程进入 Release Inbox；实际引用回收和资源退休仍由 Runtime Reactor 串行处理。
- v1 不提供 frame wait；未来新增时必须只观察/等待，不得隐式推进 Runtime Reactor。

## A. Host-pumped 单控制线程

Host 在 control thread 串行 mutation、render、poll 和 shutdown。Runtime 不创建推进线程；Validation Adapter 完全由 poll 推进。

优点：

- 状态、顺序和性能成本最确定；
- 最适合 Validation、TDD、嵌入式主循环和帧预算；
- 不需要大范围锁或回调重入规则；
- 能验证 `render` 非阻塞和 query 无副作用。

代价：

- Host 必须定期 poll；
- 多 producer 需要由 Host/SDK 汇聚到 control thread；
- Headless 阻塞便利性需要显式 pump 循环。

Phase 1 推荐并承诺实现此模型。

该选择附带架构验证条件：HostPumpDriver 必须与线程无关的 Runtime Reactor 分离；在 Host ABI v1 冻结前，使用 Validation Adapter 实现最小 DedicatedDriver spike，并复用同一套契约测试。验证目标是证明增加内部推进线程无需修改 Host C ABI、SceneVersion、FrameTicket、shutdown 和资源所有权语义。

## B. Dedicated Runtime threads

Host 仍从 control thread 提交，但 Runtime 内部线程推进编译、提交和退休。query 可跨线程，wait 可以阻塞。

优点：

- Host 不必主动推进 GPU 工作；
- Viewer、编辑器和持续渲染集成更自然；
- 后台 pipeline/asset 工作不阻塞主循环。

代价：

- 调度和完成时序更不确定；
- shutdown、device lost 和线程加入更复杂；
- TDD 需要同步原语，容易产生时序测试；
- 内部线程资源占用需要配置。

同一 ABI 可以支持该模型：poll 退化为分发完成/维护。Dedicated 成为真实能力后可以向 ABI 新增阻塞 wait；Phase 1 不提前建立该 interface。

未来 frame wait 适合 Dedicated 或“control thread 持续推进、worker thread 等待结果”的真实调用者。它不能从 Host-pumped control thread 阻塞调用；active waiter 必须参与 ticket 生命周期并在 shutdown/device lost 时被唤醒。

## C. Hybrid host-controlled submission

内部工作线程执行准备、编译或资产工作，Host 在 poll 时决定提交和完成可见点。

优点：

- 保留主循环控制，同时利用 CPU 并行；
- 可以用时间预算限制每帧尖峰；
- 适合后续 pipeline 编译、解码和 Scene commit。

代价：

- 哪些阶段由 Host 推进必须严格定义；
- worker 完成不等于 Host 可见完成；
- shutdown 需要同时 drain worker 和 GPU；
- 诊断比纯 Host-pumped 复杂。

此模型适合 Phase 2 评估。

## D. 显式多 producer submit queues

每个 producer 持有 CPU 侧 submit queue，队列内 FIFO，跨队列使用显式依赖。Runtime 将这些队列调度到内部 GPU 执行。

优点：高吞吐、竞争局部化，适合仿真和数据生成。

代价：

- queue full、跨 queue 依赖、批量原子性和 shutdown 复杂；
- ABI 函数和测试矩阵显著增加；
- 对 Phase 1 Viewer/CLI 没有真实收益。

不进入 v1。只有出现两个真实多 producer 调用者后再建立该 seam。

## 决策建议

```text
Phase 1: Host-pumped
Phase 2 candidate: Hybrid
Product integration candidate: Dedicated
Future evidence required: Explicit queues
```

ABI 的 `threading_model` 和 capability table 允许未来加入 B/C，但未实现模式在 create 时返回 `YS_ERROR_UNSUPPORTED_CAPABILITY`，不得静默回退。

## Progress Driver 验证结构

```text
Runtime Command Intake
        ↓
Runtime Reactor.advance(budget)
        ↑
        ├─ HostPumpDriver（Phase 1 正式功能）
        └─ DedicatedDriver（Validation 架构 spike）
```

Scene commit、FrameTicket 状态机、Compiled Plan 推进、完成发布、资源退休和 shutdown 状态机必须属于线程模型无关的 Runtime Reactor。Driver 只决定何时调用 `advance`，不得定义另一套对象或状态语义。
