#pragma once

#include <thread>

#include "runtime_reactor.hpp"

namespace yog_sothoth::runtime {

class RuntimeCore final {
public:
    RuntimeCore();
    ~RuntimeCore() = default;

    RuntimeCore(const RuntimeCore&) = delete;
    RuntimeCore& operator=(const RuntimeCore&) = delete;
    RuntimeCore(RuntimeCore&&) = delete;
    RuntimeCore& operator=(RuntimeCore&&) = delete;

    [[nodiscard]] RuntimePhase state() const noexcept;
    [[nodiscard]] bool shutdown(ShutdownMode mode) noexcept;
    [[nodiscard]] AdvanceResult poll(const AdvanceBudget& budget) noexcept;

private:
    // control thread 和推进状态均属于 Runtime，不暴露到公共 C ABI。
    std::thread::id control_thread_;
    RuntimeReactor reactor_;
};

} // namespace yog_sothoth::runtime
