# 实现 Validation Runtime 生命周期最小纵切

Type: task
Status: ready-for-agent
Resolution: red-ready
Blocked by: 02

## Goal

通过唯一公共 Host C ABI seam，实现 Validation + Host-pumped Runtime 的最小生命周期：`create → RUNNING → shutdown → QUIESCING → poll → STOPPED → destroy`。本 ticket 不实现 Scene、Output、Frame、GPU 或 Vulkan 行为。

## Test seam

行为测试只能包含 `include/yog_sothoth/runtime.h`，不得包含、链接或观察 Runtime 私有 C++ 类型。测试通过公开函数返回值、Runtime state 和对象生命周期判断行为。

## Acceptance

- 使用纯 C17 测试链接真实 `yog_sothoth_runtime` target；
- `YS_RUNTIME_DESC_INIT` 可以创建 Validation + Host-pumped Runtime；
- create 成功后 state 为 `YS_RUNTIME_RUNNING`；
- 首次 `YS_SHUTDOWN_DRAIN` 成功并进入 `YS_RUNTIME_QUIESCING`；
- shutdown 和 `get_state` 不隐式推进，只有 `poll` 可以推进；
- 在没有帧或 GPU 工作时，有限次 poll 最终进入 `YS_RUNTIME_STOPPED`；
- STOPPED Runtime 可以 destroy；
- 后续 red-green 循环覆盖非 STOPPED destroy、shutdown 幂等与生命周期错误；
- 所有 C ABI 入口捕获 C++ 异常，不允许异常越过 ABI；
- Runtime implementation 使用 Ticket 2 的内部 C++23 构建策略；
- 原有 ABI、工具链和 layout 测试继续通过。

## Current TDD cycle

首个 tracer bullet 只覆盖合法生命周期 happy path 和“无 poll 不推进”。`RuntimeReactor` 的空闲关闭状态机已经实现；当前预期仍在链接阶段失败，因为五个 Runtime Host C ABI 生命周期入口尚未定义。

## Comments

- 2026-07-28：用户与 Agent 明确分工；Agent 负责 Host C ABI 行为测试，用户负责 `src/runtime/**` implementation。
- 2026-07-28：已确认测试 seam 和生命周期状态语义来自 Runtime Host C ABI candidate baseline。
- 2026-07-28：首个 tracer bullet 已由 MinGW 以 C17、`-Wall -Wextra -Wpedantic -Werror` 成功编译；链接按预期失败，唯一原因是尚不存在 `yog_sothoth_runtime` library（`cannot find -lyog_sothoth_runtime`）。状态为 `RED READY`。
- 2026-07-28：已创建用户实现用的 `src/runtime/**` 骨架并接入内部 C++23 策略；静态库本身严格编译成功。红灯现已收窄为 `ys_runtime_create/get_state/shutdown/poll/destroy` 符号未定义，等待用户实现。
- 2026-07-28：线程无关推进 module 的正式领域术语确认为 Runtime Reactor；源码、CMake、规格、ADR 与 threading/interface 文档已完成一致重命名，红灯内容未改变。
- 2026-07-29：已实现 Runtime Reactor 的状态观察、shutdown 发布、模式幂等/单向升级，以及空闲 Runtime 的 `QUIESCING → DRAINING → STOPPED` 推进；私有 seam 测试仅用于本次红绿验证，未保留到仓库，正式验收继续遵循公共 Host C ABI test seam。
