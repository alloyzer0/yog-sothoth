# Host C ABI 测试 seam

Status: confirmed

## Seam

Phase 1 Runtime 的契约测试仅包含并调用 [runtime-host-c-abi.h](runtime-host-c-abi.h)。测试不得链接或访问：

- Runtime 私有 C++ 类型；
- Scene Database 容器或引用计数；
- Validation Adapter 私有状态；
- Workload Compiler 实现类型；
- Vulkan header、handle、barrier 或 timeline；
- 私有日志、allocator 或线程对象。

接口既是调用者表面，也是测试表面。内部实现整体替换后，只要 Host 可观察行为不变，测试不应修改。

## 契约测试类别

### ABI

- 公共头文件可由纯 C17 translation unit 独立编译；
- 不依赖 C++、Vulkan、窗口库或训练框架；
- 短结构、长结构、未知/重复扩展链产生确定结果；
- 每个扩展由 structure_type 唯一识别，错误类型或环链被拒绝；
- capability 集合使用无碰撞的独立 bit flags；
- 导出符号使用固定 YS_API/YS_CALL，MSVC/Clang-cl 的 C/C++ sizeof/alignof/offsetof 清单一致；
- record stream 拒绝错位、过小、过大或越界 record；
- 未知 optional 扩展被忽略，未知 required 扩展被拒绝；
- copy interface 的 required size 包含 NUL，所有非空输出缓冲均 NUL 终止；
- 失败时 out handle 清零；保留字段非零被拒绝。

### Runtime 生命周期

- create 返回有效 Runtime；
- 不支持的 backend/threading model 在 create 时失败；
- shutdown 幂等并拒绝新工作；
- drain 与 cancel-queued 行为不同；
- Host-pumped 必须 poll 才能 STOPPED；
- shutdown 和 get_state 不得隐式推进工作；
- 非 STOPPED destroy 返回错误且不破坏 Runtime；
- STOPPED destroy 成功。

### Handle 与所有权

- zero、wrong type、cross-runtime、stale generation 均被拒绝；
- 双 Runtime 相同 slot/generation 场景仍拒绝 cross-runtime handle；generation wrap 不复活 stale handle；
- commit 成功消费 transaction，随后再次使用返回 INVALID_HANDLE；
- commit 失败不消费 transaction、不产生 SceneVersion，且允许重试或 abort；
- SceneVersion release 后不能提交新帧；
- SceneVersion 提前 release 不破坏已经接受的帧；
- View destroy 不破坏在途帧捕获的 revision；
- Output destroy 后在途 ticket 仍可 query/report/map readback，旧 revision 延迟退休；
- ticket release 后 query 返回 invalid handle。
- package/profile 的 stale、wrong-type 和 cross-runtime handle 被拒绝；
- Transaction/Profile/Frame 持有 package 内部引用，Host 提前 release package 不破坏后续 commit/render；
- Frame 可以等待 QUEUED package；READY 后继续，FAILED 后继承诊断；cancel shutdown 释放等待依赖；
- Profile update 后在途帧仍使用捕获 revision，destroy 不破坏已接受帧；

### Frame 状态

- render 非阻塞返回 QUEUED ticket；
- query 不推进状态；
- poll 确定性推进合法状态；
- 终态稳定且可重复 query；
- queue full 不产生半提交 ticket；
- COMPLETED 时 Output ready；
- async failure 通过 FrameStatus 和 diagnostics id 观察。
- completed ticket 不占 execution capacity，但受 retained-result/readback 独立预算限制；
- device lost 使 queued/submitted ticket 失败并允许失败清理进入 STOPPED；
- acquire/present 的 OUTDATED、SUBOPTIMAL、SURFACE_LOST 严格匹配状态矩阵；
- v1 没有 frame wait；所有完成等待测试使用显式 poll + query。

### Offscreen readback

- 只有 COMPLETED ticket 可以 map；
- map 返回 ticket 捕获输出的正确尺寸、格式、row pitch 和数据大小，无需原 Output handle；
- 单个 ticket 重复 map 被拒绝；
- unmap 必须在 map 线程执行；
- mapped 时 frame release 返回 BUSY；
- unmap 后指针失效且 ticket 可以 release；
- shutdown 必须等待 active readback lease 结束后才能 STOPPED。

### Frame evidence

- COMPLETED ticket 可通过 Host C ABI 导出 workload、timings、metrics、resources、provenance JSON；
- `destination == NULL` 返回 required size，容量不足安全截断并返回完整大小；
- pending ticket 返回 NOT_READY；
- 报告至少保留到 ticket release，release 后查询返回 INVALID_HANDLE；
- FAILED/CANCELLED 报告标明哪些数据缺失及原因；
- 报告只读，不能用于注入 pass、资源或 barrier。
- 未请求的报告返回 NOT_CAPTURED，默认帧不启用昂贵 timestamp/resource trace；
- 终态后 report size/content 冻结，两段 copy 不发生 TOCTOU；

### Feature 与 Swapchain

- FeatureProfile 拒绝重复 kind、Grid+Neural 同时启用、错误资产类型和无 Reference 的 ErrorView；
- 空 Profile 可以执行 Validation Output，FrameRequest 的零 Profile handle 被拒绝；
- Profile 参数更新不重编译计划，拓扑更新产生一次带原因的重编译；
- Win32 Output 拒绝空 handle、错误窗口线程和未知扩展；
- resize 捕获新 revision，最小化返回 NOT_READY，surface lost 产生结构化错误；
- Runtime 自动 acquire/present；COMPLETED 表示 present 已排队；OUTDATED/SUBOPTIMAL/SURFACE_LOST 分别可观察；
- `output_get_info` 返回实际 format/color space/present mode；
- viewport rect 超出 Output 被拒绝，resize 使 history 失效；
- output_destroy 返回后不再发生 native handle 访问；未开始/已 acquire/已 present Frame 得到确定终态；

### 线程

- control-thread-only 操作从错误线程返回 WRONG_THREAD；
- query/diagnostic query 可以并发；
- transaction 不能跨线程修改；
- SceneVersion 和 FrameTicket 可以从任意 Host 线程 release；两个线程并发 release 时恰好一个成功；
- release 与 query 竞态不得崩溃，且 FrameTicket release 不取消工作；
- shutdown 必须 drain Release Inbox；
- Host-pumped 模式不创建内部推进线程；
- Runtime 不从隐式线程调用 Host。
- release/render/query/map/report race 遵循 operation pin 的线性化允许结果；
- 观察到 COMPLETED 后，readback/report/diagnostic 对观察线程可见；
- shutdown cutoff 前后命令有单调 sequence，Dedicated 与 HostPump 结果一致；
- STOPPED 允许纯 Host handle 存在，但 destroy 在 handle 未释放或 ABI 调用在途时返回 BUSY；
- Runtime instance tag 在销毁/重建后不复用；旧 Runtime handle 永不复活；
- shutdown cutoff 后 release 不再入 Inbox，STOPPED release gate 关闭最终竞态；
- STOPPED 前 GPU-backed handle detach 为 CPU-only tombstone，STOPPED 后 release 不依赖 backend；
- 短期 operation pin 不阻止 release 成功；只有 durable readback lease 返回 BUSY 且不消费 handle；

同一套参数化 Host ABI 契约测试必须至少运行于：

1. Validation + HostPumpDriver；
2. Validation + DedicatedDriver spike。

二者共同验证 render 非阻塞、SceneVersion 在途引用、FrameTicket 终态、shutdown、错误和诊断语义。Driver 特有断言分开验证：HostPumpDriver 在不 poll 时不得推进；DedicatedDriver 在不 poll 时可以最终推进。若加入 spike 需要修改公共 C ABI 或核心状态机，则 ABI 不得冻结。

### 错误

- 每次失败返回稳定 result 和完整 caller-owned error；
- 不通过字符串判断错误类别；
- Host 不提供 message buffer 时仍返回完整结构化元数据；
- message buffer 不足时安全截断、NUL 终止并返回完整 required size；
- Runtime 不保留 Host message 指针；
- 多线程错误不会互相覆盖；
- async diagnostic 至少保留到关联 ticket release。

## TDD 顺序

Review 通过后按纵向 slice 逐个 red → green：

1. 创建/关闭/销毁 Validation Runtime；
2. ABI 不兼容和详细错误；
3. 空 transaction commit 与 SceneVersion；
4. Validation Output/View；
5. AssetPackage 与 FeatureProfile 生命周期/验证；
6. render 非阻塞与 QUEUED ticket；
7. poll 推进与终态；
8. stale/cross-runtime handle；
9. 在途 SceneVersion/View/Output 生命周期；
10. drain/cancel shutdown；
11. 并发 query 与线程违规。

未来增加 `frame_wait` 时，必须新增契约测试验证“只等待、不推进”、timeout 不消费 ticket、control-thread 阻塞被拒绝、active waiter 阻止 release，以及 shutdown/device-lost 唤醒。

每个 slice 只添加一个失败测试和通过它所需的最小实现；不先批量写完整测试套件。
