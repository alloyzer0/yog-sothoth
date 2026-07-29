#include <stdint.h>
#include <stdio.h>

#include <yog_sothoth/runtime.h>

static int ys_test_check(int condition, const char* message) {
    if (condition) {
        return 1;
    }

    (void)fprintf(stderr, "runtime.lifecycle: %s\n", message);
    return 0;
}

int main(void) {
    ys_runtime_desc runtime_desc = YS_RUNTIME_DESC_INIT;
    ys_runtime* runtime = NULL;
    ys_runtime_state state = 0;

    if (!ys_test_check(
            ys_runtime_create(&runtime_desc, &runtime, NULL) == YS_OK,
            "create should accept the default Validation Host-pumped descriptor")) {
        return 1;
    }
    if (!ys_test_check(runtime != NULL, "create should return a non-null Runtime")) {
        return 1;
    }
    if (!ys_test_check(
            ys_runtime_get_state(runtime, &state, NULL) == YS_OK,
            "get_state should succeed for a live Runtime")) {
        return 1;
    }
    if (!ys_test_check(state == YS_RUNTIME_RUNNING, "new Runtime should be RUNNING")) {
        return 1;
    }

    if (!ys_test_check(
            ys_runtime_shutdown(runtime, YS_SHUTDOWN_DRAIN, NULL) == YS_OK,
            "shutdown(DRAIN) should be accepted")) {
        return 1;
    }
    if (!ys_test_check(
            ys_runtime_get_state(runtime, &state, NULL) == YS_OK,
            "get_state should observe shutdown state")) {
        return 1;
    }
    if (!ys_test_check(
            state == YS_RUNTIME_QUIESCING,
            "first shutdown should publish QUIESCING without advancing work")) {
        return 1;
    }

    /* get_state 只能观察，连续查询不得代替 Host-pumped poll 推进生命周期。 */
    if (!ys_test_check(
            ys_runtime_get_state(runtime, &state, NULL) == YS_OK,
            "repeated get_state should succeed")) {
        return 1;
    }
    if (!ys_test_check(
            state == YS_RUNTIME_QUIESCING,
            "get_state must not advance a Host-pumped Runtime")) {
        return 1;
    }

    ys_poll_desc poll_desc = YS_POLL_DESC_INIT;
    poll_desc.max_work_items = UINT32_C(16);
    poll_desc.time_budget_ns = UINT64_C(1000000);

    /* 当前无 Frame/GPU 工作；允许 implementation 用多个有限步骤完成 drain。 */
    for (uint32_t attempt = 0; attempt < UINT32_C(8) && state != YS_RUNTIME_STOPPED; ++attempt) {
        ys_poll_result poll_result = YS_POLL_RESULT_INIT;
        if (!ys_test_check(
                ys_runtime_poll(runtime, &poll_desc, &poll_result, NULL) == YS_OK,
                "poll should advance an accepted shutdown")) {
            return 1;
        }
        if (!ys_test_check(
                ys_runtime_get_state(runtime, &state, NULL) == YS_OK,
                "get_state should observe progress published by poll")) {
            return 1;
        }
    }

    if (!ys_test_check(
            state == YS_RUNTIME_STOPPED,
            "an idle Runtime should reach STOPPED after bounded polling")) {
        return 1;
    }
    if (!ys_test_check(
            ys_runtime_destroy(runtime, NULL) == YS_OK,
            "destroy should accept a STOPPED Runtime")) {
        return 1;
    }

    return 0;
}
