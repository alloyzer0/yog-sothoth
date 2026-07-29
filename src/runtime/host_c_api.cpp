#include <yog_sothoth/runtime.h>

#include "runtime_core.hpp"

// opaque C handle 只包装私有 C++23 RuntimeCore，不向 Host 暴露内部布局。
struct ys_runtime {
    yog_sothoth::runtime::RuntimeCore core;
};

YS_API ys_result YS_CALL ys_runtime_create(
    const ys_runtime_desc* desc,
    ys_runtime** out_runtime,
    ys_error_info* error)
{
    static_cast<void>(desc);
    static_cast<void>(out_runtime);
    static_cast<void>(error);

    // TODO(ticket-3): 验证 Runtime descriptor、创建 opaque Runtime 并拦截所有异常。
    return YS_ERROR_INTERNAL;
}

YS_API ys_result YS_CALL ys_runtime_get_state(
    ys_runtime* runtime,
    ys_runtime_state* out_state,
    ys_error_info* error)
{
    static_cast<void>(runtime);
    static_cast<void>(out_state);
    static_cast<void>(error);

    // TODO(ticket-3): 观察 RuntimeCore state 并显式映射为公共 Runtime state。
    return YS_ERROR_INTERNAL;
}

YS_API ys_result YS_CALL ys_runtime_shutdown(
    ys_runtime* runtime,
    ys_shutdown_mode mode,
    ys_error_info* error)
{
    static_cast<void>(runtime);
    static_cast<void>(mode);
    static_cast<void>(error);

    // TODO(ticket-3): 映射 shutdown mode，并向 RuntimeCore 提交关闭意图。
    return YS_ERROR_INTERNAL;
}

YS_API ys_result YS_CALL ys_runtime_poll(
    ys_runtime* runtime,
    const ys_poll_desc* desc,
    ys_poll_result* out_result,
    ys_error_info* error)
{
    static_cast<void>(runtime);
    static_cast<void>(desc);
    static_cast<void>(out_result);
    static_cast<void>(error);

    // TODO(ticket-3): 映射推进预算、调用 RuntimeCore::poll 并填写公共 poll result。
    return YS_ERROR_INTERNAL;
}

YS_API ys_result YS_CALL ys_runtime_destroy(ys_runtime* runtime, ys_error_info* error)
{
    static_cast<void>(runtime);
    static_cast<void>(error);

    // TODO(ticket-3): 仅销毁已进入 STOPPED 的 Runtime，并拦截所有异常。
    return YS_ERROR_INTERNAL;
}
