#include <yog_sothoth/runtime.h>

#include "runtime_core.hpp"

// opaque C handle 只包装私有 C++23 RuntimeCore，不向 Host 暴露内部布局。
struct ys_runtime {
    yog_sothoth::runtime::RuntimeCore core;
};

// TODO(ticket-3): 在此实现五个 Runtime 生命周期 C ABI 入口并拦截所有 C++ 异常。
