# Yog-Sothoth Rendering Runtime

Yog-Sothoth 是一个可嵌入、可验证的实时渲染 Runtime；领域语言区分稳定 Host interface、线程无关推进机制和具体驱动策略。

## Language

**Runtime Reactor**:
线程无关的 Runtime 推进机制，在有界预算内处理已接受工作、发布生命周期状态并退休资源。Progress Driver 决定何时驱动它，但不拥有另一套状态语义。
_Avoid_: Progress Engine, scheduler, event loop

**Progress Driver**:
决定 Runtime Reactor 在何时、由哪个线程调用 `advance` 的驱动策略。
_Avoid_: Runtime scheduler, worker thread
