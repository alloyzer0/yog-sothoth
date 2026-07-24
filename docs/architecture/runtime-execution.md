# Runtime 帧执行时序

## 正常帧

```mermaid
sequenceDiagram
    autonumber
    participant H as Host Adapter
    participant R as Render Runtime
    participant S as Scene Database
    participant A as Asset Runtime
    participant W as Workload Compiler
    participant G as GPU Runtime
    participant O as Output Adapter
    participant D as Diagnostics

    H->>R: ys_scene_commit(transaction)
    R->>A: resolve(package assets)
    A-->>R: stable asset handles
    R->>S: commit(transaction, asset handles)
    S-->>R: immutable SceneVersion
    R-->>H: SceneVersion

    H->>R: ys_render(FrameRequest)
    R->>S: retain(SceneVersion)
    S-->>R: Render Scene Snapshot
    R->>O: resolve Output contract
    O-->>R: format/extent/usage
    R->>W: findOrCompile(FeatureSet, Output, Capabilities)

    alt Compiled Plan cache hit
        W-->>R: cached Compiled Plan
    else Graph structure changed
        W->>W: validate, cull, plan lifetime/barriers
        W->>D: record compile reason and plan
        W-->>R: new Compiled Plan
    end

    R->>G: submit(plan, snapshot, frame data)
    G->>O: acquire/write output
    G->>D: timestamp and resource events
    G-->>R: timeline value
    R-->>H: FrameTicket (non-blocking)

    H->>R: ys_frame_query(ticket)
    R->>G: query timeline
    G-->>R: pending/completed
    R-->>H: FrameStatus + diagnostics id

    opt completed
        R->>S: release SceneVersion
        R->>G: retire deferred resources
        R->>O: expose present/readback result
    end
```

## 线程与阻塞约束

- `ys_render` 只提交工作并返回 `FrameTicket`，不得等待 GPU 完成。
- SceneTransaction 可以在宿主线程构建；commit 在 Phase 1 串行进入 Runtime。
- Vulkan Adapter 可以拥有内部提交线程，但不得从未声明线程调用宿主回调。
- Headless readback 只有在对应 ticket 完成后才允许映射。
- 稳定帧不得进行文件 I/O、shader 编译、pipeline 创建或 CPU 阻塞等待 GPU。

## Compiled Plan 稳定性

以下变化不触发重编译：

- 相机矩阵；
- transform；
- 材质参数；
- SceneVersion 内容；
- Neural/Grid 参数值。

以下变化可以触发重编译，并必须记录原因：

- Feature 拓扑；
- Output 格式或尺寸类别；
- capability/fallback 路径；
- pass 资源契约；
- shader/pipeline package 兼容性变化。

## 故障路径

任何阶段失败均返回结构化错误：错误码、所属子系统、稳定消息和 diagnostics id。Runtime 必须保证失败请求不会部分提交场景版本，也不会泄漏或提前释放在途资源。

