#include "runtime_reactor.hpp"

namespace yog_sothoth::runtime {

RuntimePhase RuntimeReactor::phase() const noexcept
{
    return phase_.load(std::memory_order_acquire);
}

bool RuntimeReactor::request_shutdown(ShutdownMode mode) noexcept
{
    auto current = phase_.load(std::memory_order_acquire);
    if (current == RuntimePhase::running) {
        // 先写入关闭策略，再以 release 发布 QUIESCING；advance 的 acquire
        // 观察到 QUIESCING 后，才能安全读取对应的 shutdown mode。
        shutdown_mode_.store(mode, std::memory_order_relaxed);

        auto expected = RuntimePhase::running;
        if (phase_.compare_exchange_strong(
                expected,
                RuntimePhase::quiescing,
                std::memory_order_release,
                std::memory_order_acquire)) {
            return true;
        }

        // CAS 失败时 expected 已更新为实际状态，继续按幂等规则判断。
        current = expected;
    }

    if (current != RuntimePhase::quiescing && current != RuntimePhase::draining) {
        return false;
    }

    auto accepted_mode = shutdown_mode_.load(std::memory_order_acquire);
    if (accepted_mode == mode) {
        return true;
    }

    // CANCEL_QUEUED 比 DRAIN 更强，只允许单向升级，避免已取消工作被重新解释为 drain。
    if (accepted_mode == ShutdownMode::drain && mode == ShutdownMode::cancel_queued) {
        return shutdown_mode_.compare_exchange_strong(
            accepted_mode,
            ShutdownMode::cancel_queued,
            std::memory_order_acq_rel,
            std::memory_order_acquire);
    }

    return false;
}

AdvanceResult RuntimeReactor::advance(const AdvanceBudget& budget) noexcept
{
    // 当前纵切尚无可调度 work item，预算只保留为后续队列推进契约。
    static_cast<void>(budget);

    const auto current = phase_.load(std::memory_order_acquire);
    if (current == RuntimePhase::quiescing) {
        // QUIESCING 表示关闭意图已经发布；进入 DRAINING 后不再接受新工作。
        phase_.store(RuntimePhase::draining, std::memory_order_release);

        // 当前 Validation Runtime 没有 Frame、GPU 或资源退休工作，因此
        // 同一次 advance 即可证明 drain 完成并发布 STOPPED。
        phase_.store(RuntimePhase::stopped, std::memory_order_release);
    }

    return AdvanceResult{};
}
} // namespace yog_sothoth::runtime
