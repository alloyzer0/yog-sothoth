#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <yog_sothoth/runtime.h>

/*
 * Windows x64 candidate baseline。字面量来自已审核的首次 MinGW 采样，
 * 不是用被测表达式重新计算预期值；ABI 变更必须显式更新本文件和 JSON 基线。
 */
_Static_assert(sizeof(void*) == 8, "Host ABI v1 仅验收 Windows x64");
_Static_assert(sizeof(ys_struct_header) == 24, "ys_struct_header size changed");
_Static_assert(_Alignof(ys_struct_header) == 8, "ys_struct_header alignment changed");
_Static_assert(offsetof(ys_struct_header, next) == 16, "ys_struct_header.next offset changed");

_Static_assert(sizeof(ys_runtime_desc) == 72, "ys_runtime_desc size changed");
_Static_assert(_Alignof(ys_runtime_desc) == 8, "ys_runtime_desc alignment changed");
_Static_assert(
    offsetof(ys_runtime_desc, device_id) == 56,
    "ys_runtime_desc.device_id offset changed");

_Static_assert(sizeof(ys_error_info) == 96, "ys_error_info size changed");
_Static_assert(_Alignof(ys_error_info) == 8, "ys_error_info alignment changed");
_Static_assert(offsetof(ys_error_info, message) == 72, "ys_error_info.message offset changed");

_Static_assert(sizeof(ys_frame_request) == 80, "ys_frame_request size changed");
_Static_assert(_Alignof(ys_frame_request) == 8, "ys_frame_request alignment changed");
_Static_assert(
    offsetof(ys_frame_request, feature_profile) == 48,
    "ys_frame_request.feature_profile offset changed");

_Static_assert(sizeof(ys_frame_status) == 56, "ys_frame_status size changed");
_Static_assert(_Alignof(ys_frame_status) == 8, "ys_frame_status alignment changed");
_Static_assert(
    offsetof(ys_frame_status, submit_sequence) == 48,
    "ys_frame_status.submit_sequence offset changed");

int main(void)
{
    printf("{\n");
    printf("  \"pointer_size\": %zu,\n", sizeof(void*));
    printf(
        "  \"ys_struct_header\": {\"size\": %zu, \"align\": %zu, \"next\": %zu},\n",
        sizeof(ys_struct_header),
        _Alignof(ys_struct_header),
        offsetof(ys_struct_header, next));
    printf(
        "  \"ys_runtime_desc\": {\"size\": %zu, \"align\": %zu, \"device_id\": %zu},\n",
        sizeof(ys_runtime_desc),
        _Alignof(ys_runtime_desc),
        offsetof(ys_runtime_desc, device_id));
    printf(
        "  \"ys_error_info\": {\"size\": %zu, \"align\": %zu, \"message\": %zu},\n",
        sizeof(ys_error_info),
        _Alignof(ys_error_info),
        offsetof(ys_error_info, message));
    printf(
        "  \"ys_frame_request\": {\"size\": %zu, \"align\": %zu, \"feature_profile\": %zu},\n",
        sizeof(ys_frame_request),
        _Alignof(ys_frame_request),
        offsetof(ys_frame_request, feature_profile));
    printf(
        "  \"ys_frame_status\": {\"size\": %zu, \"align\": %zu, \"submit_sequence\": %zu}\n",
        sizeof(ys_frame_status),
        _Alignof(ys_frame_status),
        offsetof(ys_frame_status, submit_sequence));
    printf("}\n");
    return 0;
}
