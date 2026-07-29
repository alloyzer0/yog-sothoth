#include "runtime_core.hpp"

namespace yog_sothoth::runtime {

// TODO(ticket-3): 捕获创建线程，并把 state/shutdown/poll 委托给 Runtime Reactor。

RuntimeCore::RuntimeCore()
{
}
} // namespace yog_sothoth::runtime
