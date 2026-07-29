#pragma once

#include <atomic>
#include <cstdint>

namespace yog_sothoth::runtime {

enum class RuntimePhase : std::uint32_t {
    running,
    quiescing,
    draining,
    stopped,
    failed,
};

enum class ShutdownMode : std::uint32_t {
    drain,
    cancel_queued,
};

struct AdvanceBudget {
    std::uint32_t max_work_items{};
    std::uint64_t time_budget_ns{};
};

struct AdvanceResult {
    std::uint32_t work_items_processed{};
    bool more_work_pending{};
};

class RuntimeReactor final {
public:
    RuntimeReactor() noexcept = default;

    // 只观察已发布状态；不得在 query 路径中隐式推进 Runtime。
    [[nodiscard]] RuntimePhase phase() const noexcept;

    // 只提交关闭意图并发布 QUIESCING，不执行 drain 或资源退休。
    [[nodiscard]] bool request_shutdown(ShutdownMode mode) noexcept;

    // 唯一推进入口；预算限制开始新 work item，不中断已开始的工作。
    [[nodiscard]] AdvanceResult advance(const AdvanceBudget& budget) noexcept;

private:
    // 状态由 Runtime Reactor 统一发布；Progress Driver 只能驱动 advance。
    std::atomic<RuntimePhase> phase_{RuntimePhase::running};

    // shutdown mode 可在关闭期间从 DRAIN 单向升级为 CANCEL_QUEUED；
    // 使用原子值避免未来 Dedicated Driver 与 control thread 之间产生竞态。
    std::atomic<ShutdownMode> shutdown_mode_{ShutdownMode::drain};
};

} // namespace yog_sothoth::runtime
