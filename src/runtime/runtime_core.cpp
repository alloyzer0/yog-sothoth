#include "runtime_core.hpp"

namespace yog_sothoth::runtime {

RuntimeCore::RuntimeCore()
    : control_thread_(std::this_thread::get_id())
{
}

RuntimePhase RuntimeCore::state() const noexcept
{
    return reactor_.phase();
}

bool RuntimeCore::shutdown(ShutdownMode mode) noexcept
{
    return reactor_.request_shutdown(mode);
}

AdvanceResult RuntimeCore::poll(const AdvanceBudget& budget) noexcept
{
    return reactor_.advance(budget);
}
} // namespace yog_sothoth::runtime
