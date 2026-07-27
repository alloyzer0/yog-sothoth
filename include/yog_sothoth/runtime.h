#ifndef YOG_SOTHOTH_RUNTIME_HOST_C_ABI_H
#define YOG_SOTHOTH_RUNTIME_HOST_C_ABI_H

/*
 * Yog-Sothoth Runtime Host C ABI v1 candidate baseline。
 * 本文件是 Host C ABI 的唯一机器可读真源；当前不提供 Runtime 实现。
 */

#include <stdint.h>

#if defined(_WIN32)
#  if defined(YS_STATIC)
#    define YS_API
#  elif defined(YS_BUILD_DLL)
#    define YS_API __declspec(dllexport)
#  else
#    define YS_API __declspec(dllimport)
#  endif
#  define YS_CALL __cdecl
#else
#  define YS_API __attribute__((visibility("default")))
#  define YS_CALL
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define YS_ABI_MAJOR_1 UINT32_C(1)
#define YS_ABI_VERSION_1 YS_ABI_MAJOR_1
#define YS_INVALID_HANDLE UINT64_C(0)
#define YS_STRUCTURE_REQUIRED_BIT UINT32_C(0x80000000)
#define YS_STRUCTURE_TYPE_ID(value) ((value) & ~YS_STRUCTURE_REQUIRED_BIT)

/* v1 中所有名为 flags/reserved 的输入字段必须为 0，除非相邻注释另有定义。 */

typedef struct ys_runtime ys_runtime;
typedef uint64_t ys_scene_transaction;
typedef uint64_t ys_scene_version;
typedef uint64_t ys_view;
typedef uint64_t ys_output;
typedef uint64_t ys_asset_package;
typedef uint64_t ys_feature_profile;
typedef uint64_t ys_frame_ticket;
typedef uint64_t ys_diagnostics_id;
typedef uint64_t ys_object_id;

typedef struct ys_blob_view {
    const void* data;
    uint64_t byte_size;
} ys_blob_view;

typedef struct ys_abi_info {
    uint32_t runtime_version_major;
    uint32_t runtime_version_minor;
    uint32_t runtime_version_patch;
    uint32_t abi_major_min;
    uint32_t abi_major_max;
    uint32_t reserved[3];
} ys_abi_info;

/* ys_query_abi 只写入 min(out_info_size, sizeof(ys_abi_info)) 字节。 */

typedef uint32_t ys_structure_type;
enum {
    YS_STRUCTURE_ERROR_INFO = 1,
    YS_STRUCTURE_RUNTIME_DESC = 2,
    YS_STRUCTURE_RUNTIME_CAPABILITIES = 3,
    YS_STRUCTURE_POLL_DESC = 4,
    YS_STRUCTURE_POLL_RESULT = 5,
    YS_STRUCTURE_SCENE_COMMIT_DESC = 6,
    YS_STRUCTURE_VIEW_DESC = 7,
    YS_STRUCTURE_OUTPUT_DESC = 8,
    YS_STRUCTURE_FRAME_REQUEST = 9,
    YS_STRUCTURE_FRAME_STATUS = 10,
    YS_STRUCTURE_MAPPED_READBACK = 11,
    YS_STRUCTURE_DIAGNOSTIC_INFO = 12,
    YS_STRUCTURE_ASSET_PACKAGE_DESC = 13,
    YS_STRUCTURE_FEATURE_PROFILE_DESC = 14,
    YS_STRUCTURE_OUTPUT_INFO = 15,
    YS_STRUCTURE_DEVICE_INFO = 16,
    YS_STRUCTURE_OUTPUT_CAPABILITIES = 17,
    YS_STRUCTURE_OUTPUT_WIN32_SWAPCHAIN_EXT = 1001
};

typedef struct ys_struct_header {
    uint32_t struct_size;
    uint32_t abi_version;
    ys_structure_type structure_type;
    uint32_t reserved;
    const void* next;
} ys_struct_header;

typedef uint32_t ys_result;
enum {
    YS_OK = 0,
    YS_STATUS_NOT_READY = 1,
    YS_STATUS_OUTPUT_OUTDATED = 2,
    YS_STATUS_OUTPUT_SUBOPTIMAL = 3,
    YS_ERROR_INVALID_ARGUMENT = 0x1001,
    YS_ERROR_INCOMPATIBLE_ABI = 0x1002,
    YS_ERROR_INVALID_HANDLE = 0x1003,
    YS_ERROR_WRONG_THREAD = 0x1004,
    YS_ERROR_INVALID_STATE = 0x1005,
    YS_ERROR_UNSUPPORTED_CAPABILITY = 0x1006,
    YS_ERROR_QUEUE_FULL = 0x1007,
    YS_ERROR_VALIDATION_FAILED = 0x1008,
    YS_ERROR_DEVICE_LOST = 0x1009,
    YS_ERROR_OUT_OF_MEMORY = 0x100A,
    YS_ERROR_IO = 0x100B,
    YS_ERROR_DATA_CORRUPT = 0x100C,
    YS_ERROR_BUSY = 0x100D,
    YS_ERROR_SURFACE_LOST = 0x100E,
    YS_ERROR_BUFFER_TOO_SMALL = 0x100F,
    YS_ERROR_NOT_CAPTURED = 0x1010,
    YS_ERROR_RESULT_CAPACITY = 0x1011,
    YS_ERROR_INTERNAL = 0x10FF
};

typedef uint32_t ys_error_severity;
enum {
    YS_SEVERITY_INFO = 0,
    YS_SEVERITY_WARNING = 1,
    YS_SEVERITY_ERROR = 2,
    YS_SEVERITY_FATAL = 3
};

typedef uint32_t ys_subsystem;
enum {
    YS_SUBSYSTEM_HOST_ABI = 1,
    YS_SUBSYSTEM_RUNTIME = 2,
    YS_SUBSYSTEM_SCENE = 3,
    YS_SUBSYSTEM_WORKLOAD = 4,
    YS_SUBSYSTEM_OUTPUT = 5,
    YS_SUBSYSTEM_GPU = 6,
    YS_SUBSYSTEM_ASSET = 7,
    YS_SUBSYSTEM_VALIDATION = 8
};

typedef struct ys_error_info {
    /* Host 输入：header、message、message_capacity；Runtime 不修改指针和容量。 */
    ys_struct_header header;
    ys_result code;
    ys_error_severity severity;
    ys_subsystem subsystem;
    uint32_t function_id;
    uint32_t argument_index;
    uint32_t object_type;
    uint32_t reserved;
    uint64_t related_handle;
    ys_diagnostics_id diagnostics_id;
    char* message;
    uint64_t message_capacity;
    uint64_t message_required_size;
} ys_error_info;

#define YS_ERROR_INFO_INIT \
    { { (uint32_t)sizeof(ys_error_info), YS_ABI_VERSION_1, YS_STRUCTURE_ERROR_INFO, 0, 0 }, \
      YS_OK, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }

typedef uint32_t ys_backend_kind;
enum {
    YS_BACKEND_VALIDATION = 1,
    YS_BACKEND_VULKAN = 2
};

enum {
    YS_BACKEND_FLAG_VALIDATION = 1u << 0,
    YS_BACKEND_FLAG_VULKAN = 1u << 1
};

typedef uint32_t ys_threading_model;
enum {
    YS_THREADING_HOST_PUMPED = 1,
    YS_THREADING_DEDICATED = 2,
    YS_THREADING_HYBRID = 3
};

enum {
    YS_THREADING_FLAG_HOST_PUMPED = 1u << 0,
    YS_THREADING_FLAG_DEDICATED = 1u << 1,
    YS_THREADING_FLAG_HYBRID = 1u << 2
};

typedef struct ys_runtime_desc {
    ys_struct_header header;
    ys_backend_kind backend;
    ys_threading_model threading_model;
    uint32_t max_frames_in_flight;
    uint32_t max_retained_frames;
    uint32_t worker_thread_count;
    uint32_t reserved0;
    uint64_t max_readback_bytes;
    uint64_t device_id;
    uint32_t flags;
    uint32_t reserved;
} ys_runtime_desc;

#define YS_RUNTIME_DESC_INIT \
    { { (uint32_t)sizeof(ys_runtime_desc), YS_ABI_VERSION_1, YS_STRUCTURE_RUNTIME_DESC, 0, 0 }, \
      YS_BACKEND_VALIDATION, YS_THREADING_HOST_PUMPED, 3, 16, 0, 0, \
      UINT64_C(268435456), 0, 0, 0 }

typedef struct ys_runtime_capabilities {
    ys_struct_header header;
    uint32_t supported_backends;
    uint32_t supported_threading_models;
    uint32_t supported_outputs;
    uint32_t max_frames_in_flight;
    uint32_t package_version_min;
    uint32_t package_version_max;
    uint32_t reserved;
} ys_runtime_capabilities;

#define YS_RUNTIME_CAPABILITIES_INIT \
    { { (uint32_t)sizeof(ys_runtime_capabilities), YS_ABI_VERSION_1, YS_STRUCTURE_RUNTIME_CAPABILITIES, 0, 0 }, \
      0, 0, 0, 0, 0, 0, 0 }

typedef uint32_t ys_runtime_state;
enum {
    YS_RUNTIME_RUNNING = 1,
    YS_RUNTIME_QUIESCING = 2,
    YS_RUNTIME_DRAINING = 3,
    YS_RUNTIME_STOPPED = 4,
    YS_RUNTIME_FAILED = 5
};

typedef uint32_t ys_shutdown_mode;
enum {
    YS_SHUTDOWN_DRAIN = 1,
    YS_SHUTDOWN_CANCEL_QUEUED = 2
};

typedef struct ys_poll_desc {
    ys_struct_header header;
    uint32_t max_work_items;
    uint32_t reserved;
    uint64_t time_budget_ns;
} ys_poll_desc;

#define YS_POLL_DESC_INIT \
    { { (uint32_t)sizeof(ys_poll_desc), YS_ABI_VERSION_1, YS_STRUCTURE_POLL_DESC, 0, 0 }, 0, 0, 0 }

typedef struct ys_poll_result {
    ys_struct_header header;
    uint32_t work_items_processed;
    uint32_t frames_completed;
    uint32_t objects_retired;
    uint32_t more_work_pending;
    uint64_t suggested_poll_ns;
    uint64_t budget_overshoot_ns;
    uint32_t blocked_reason;
    uint32_t reserved;
} ys_poll_result;

#define YS_POLL_RESULT_INIT \
    { { (uint32_t)sizeof(ys_poll_result), YS_ABI_VERSION_1, YS_STRUCTURE_POLL_RESULT, 0, 0 }, \
      0, 0, 0, 0, 0, 0, 0, 0 }

typedef struct ys_scene_commit_desc {
    ys_struct_header header;
    uint32_t flags;
    uint32_t reserved;
} ys_scene_commit_desc;

#define YS_SCENE_COMMIT_DESC_INIT \
    { { (uint32_t)sizeof(ys_scene_commit_desc), YS_ABI_VERSION_1, YS_STRUCTURE_SCENE_COMMIT_DESC, 0, 0 }, 0, 0 }

typedef uint32_t ys_scene_command_kind;
enum {
    YS_SCENE_CREATE_OBJECT = 1,
    YS_SCENE_DESTROY_OBJECT = 2,
    YS_SCENE_SET_TRANSFORM = 3,
    YS_SCENE_SET_MESH = 4,
    YS_SCENE_SET_MATERIAL = 5,
    YS_SCENE_SET_LIGHT = 6
};

typedef struct ys_asset_ref {
    ys_asset_package package;
    uint64_t asset_id;
} ys_asset_ref;

typedef uint32_t ys_light_kind;
enum {
    YS_LIGHT_DIRECTIONAL = 1,
    YS_LIGHT_POINT = 2,
    YS_LIGHT_SPOT = 3,
    YS_LIGHT_AREA_RECT = 4
};

typedef struct ys_scene_command {
    uint32_t record_size;
    ys_scene_command_kind kind;
    ys_object_id object;
    union {
        struct {
            uint32_t flags;
            uint32_t reserved;
        } create_object;
        struct {
            float world_from_local[16];
        } set_transform;
        struct {
            ys_asset_ref mesh;
        } set_mesh;
        struct {
            ys_asset_ref material;
        } set_material;
        struct {
            ys_light_kind kind;
            uint32_t reserved;
            float color_linear[3];
            float intensity;
            float range;
            float inner_cone_radians;
            float outer_cone_radians;
            float area_width;
            float area_height;
        } set_light;
    } payload;
} ys_scene_command;

typedef struct ys_asset_package_desc {
    ys_struct_header header;
    /* Host-owned immutable bytes，仅在 load 调用期间借用。 */
    ys_blob_view package_bytes;
    uint32_t flags;
    uint32_t reserved;
} ys_asset_package_desc;

typedef uint32_t ys_asset_package_state;
enum {
    YS_ASSET_PACKAGE_QUEUED = 1,
    YS_ASSET_PACKAGE_READY = 2,
    YS_ASSET_PACKAGE_FAILED = 3
};

#define YS_ASSET_PACKAGE_DESC_INIT \
    { { (uint32_t)sizeof(ys_asset_package_desc), YS_ABI_VERSION_1, YS_STRUCTURE_ASSET_PACKAGE_DESC, 0, 0 }, \
      {0, 0}, 0, 0 }

typedef struct ys_view_desc {
    ys_struct_header header;
    float view_from_world[16];
    float clip_from_view[16];
    int32_t viewport_x;
    int32_t viewport_y;
    uint32_t viewport_width;
    uint32_t viewport_height;
    uint32_t flags;
    uint32_t reserved;
} ys_view_desc;

#define YS_VIEW_DESC_INIT \
    { { (uint32_t)sizeof(ys_view_desc), YS_ABI_VERSION_1, YS_STRUCTURE_VIEW_DESC, 0, 0 }, \
      {0}, {0}, 0, 0, 0, 0, 0, 0 }

typedef uint32_t ys_output_kind;
enum {
    YS_OUTPUT_VALIDATION = 1,
    YS_OUTPUT_OFFSCREEN_READBACK = 2,
    YS_OUTPUT_SWAPCHAIN = 3
};

enum {
    YS_OUTPUT_FLAG_VALIDATION = 1u << 0,
    YS_OUTPUT_FLAG_OFFSCREEN_READBACK = 1u << 1,
    YS_OUTPUT_FLAG_SWAPCHAIN = 1u << 2
};

typedef uint32_t ys_pixel_format;
enum {
    YS_FORMAT_RGBA8_UNORM = 1,
    YS_FORMAT_RGBA16_FLOAT = 2
};

enum {
    YS_FORMAT_FLAG_RGBA8_UNORM = 1u << 0,
    YS_FORMAT_FLAG_RGBA16_FLOAT = 1u << 1
};

typedef uint32_t ys_color_space;
enum {
    YS_COLOR_SPACE_LINEAR = 1,
    YS_COLOR_SPACE_SRGB = 2
};

enum {
    YS_COLOR_SPACE_FLAG_LINEAR = 1u << 0,
    YS_COLOR_SPACE_FLAG_SRGB = 1u << 1
};

typedef uint32_t ys_present_mode;
enum {
    YS_PRESENT_FIFO = 1,
    YS_PRESENT_MAILBOX = 2,
    YS_PRESENT_IMMEDIATE = 3
};

enum {
    YS_PRESENT_FLAG_FIFO = 1u << 0,
    YS_PRESENT_FLAG_MAILBOX = 1u << 1,
    YS_PRESENT_FLAG_IMMEDIATE = 1u << 2
};

typedef struct ys_output_desc {
    ys_struct_header header;
    ys_output_kind kind;
    uint32_t width;
    uint32_t height;
    ys_pixel_format format;
    ys_color_space color_space;
    ys_present_mode present_mode;
    uint32_t flags;
    uint32_t reserved;
} ys_output_desc;

#define YS_OUTPUT_DESC_INIT \
    { { (uint32_t)sizeof(ys_output_desc), YS_ABI_VERSION_1, YS_STRUCTURE_OUTPUT_DESC, 0, 0 }, \
      0, 0, 0, 0, 0, 0, 0, 0 }

typedef struct ys_output_info {
    ys_struct_header header;
    uint32_t width;
    uint32_t height;
    ys_pixel_format actual_format;
    ys_color_space actual_color_space;
    ys_present_mode actual_present_mode;
    uint32_t flags;
    uint64_t revision;
} ys_output_info;

#define YS_OUTPUT_INFO_INIT \
    { { (uint32_t)sizeof(ys_output_info), YS_ABI_VERSION_1, YS_STRUCTURE_OUTPUT_INFO, 0, 0 }, \
      0, 0, 0, 0, 0, 0, 0 }

typedef struct ys_device_info {
    ys_struct_header header;
    uint64_t device_id;
    ys_backend_kind backend;
    uint32_t vendor_id;
    uint32_t device_id_native;
    uint32_t device_type;
    char name_utf8[256];
} ys_device_info;

#define YS_DEVICE_INFO_INIT \
    { { (uint32_t)sizeof(ys_device_info), YS_ABI_VERSION_1, YS_STRUCTURE_DEVICE_INFO, 0, 0 }, \
      0, 0, 0, 0, 0, {0} }

typedef struct ys_output_capabilities {
    ys_struct_header header;
    uint32_t supported_formats;
    uint32_t supported_color_spaces;
    uint32_t supported_present_modes;
    uint32_t min_width;
    uint32_t min_height;
    uint32_t max_width;
    uint32_t max_height;
    uint32_t flags;
} ys_output_capabilities;

#define YS_OUTPUT_CAPABILITIES_INIT \
    { { (uint32_t)sizeof(ys_output_capabilities), YS_ABI_VERSION_1, YS_STRUCTURE_OUTPUT_CAPABILITIES, 0, 0 }, \
      0, 0, 0, 0, 0, 0, 0, 0 }

/* Win32 Swapchain Output 的版本化扩展；基础 ABI 不依赖 windows.h。 */
typedef struct ys_output_win32_swapchain_ext {
    ys_struct_header header;
    /* 两者由 Host 拥有，窗口必须属于 Runtime control thread。 */
    void* hwnd;
    void* hinstance;
} ys_output_win32_swapchain_ext;

#define YS_OUTPUT_WIN32_SWAPCHAIN_EXT_INIT \
    { { (uint32_t)sizeof(ys_output_win32_swapchain_ext), YS_ABI_VERSION_1, \
        YS_STRUCTURE_OUTPUT_WIN32_SWAPCHAIN_EXT, 0, 0 }, 0, 0 }

typedef uint32_t ys_feature_kind;
enum {
    YS_FEATURE_DIRECT_LIGHTING = 1,
    YS_FEATURE_IRRADIANCE_GRID = 2,
    YS_FEATURE_NEURAL_IRRADIANCE = 3,
    YS_FEATURE_REFERENCE_IMAGE = 4,
    YS_FEATURE_ERROR_VIEW = 5
};

typedef struct ys_feature_config {
    uint32_t record_size;
    ys_feature_kind kind;
    uint32_t enabled;
    uint32_t reserved;
    union {
        struct {
            float intensity_scale;
            float reserved_f32[3];
        } direct_lighting;
        struct {
            ys_asset_ref grid;
            float intensity_scale;
            float reserved_f32[3];
        } irradiance_grid;
        struct {
            ys_asset_ref model;
            float intensity_scale;
            float reserved_f32[3];
        } neural_irradiance;
        struct {
            ys_asset_ref image;
        } reference_image;
        struct {
            uint32_t metric_flags;
            float heatmap_scale;
            float reserved_f32[2];
        } error_view;
    } settings;
} ys_feature_config;

typedef struct ys_feature_profile_desc {
    ys_struct_header header;
    const void* feature_records;
    uint64_t feature_records_size;
    uint32_t flags;
    uint32_t reserved;
} ys_feature_profile_desc;

/*
 * v1 中每种 enabled Feature 最多一个；Grid 与 Neural 互斥；
 * ErrorView 要求 ReferenceImage 以及 Grid/Neural 之一。所有 flags 必须为 0。
 */

#define YS_FEATURE_PROFILE_DESC_INIT \
    { { (uint32_t)sizeof(ys_feature_profile_desc), YS_ABI_VERSION_1, YS_STRUCTURE_FEATURE_PROFILE_DESC, 0, 0 }, \
      0, 0, 0, 0 }

typedef struct ys_frame_request {
    ys_struct_header header;
    ys_scene_version scene;
    ys_view view;
    ys_output output;
    ys_feature_profile feature_profile;
    uint64_t user_tag;
    uint32_t report_capture_flags;
    uint32_t flags;
    uint32_t reserved;
} ys_frame_request;

#define YS_FRAME_REQUEST_INIT \
    { { (uint32_t)sizeof(ys_frame_request), YS_ABI_VERSION_1, YS_STRUCTURE_FRAME_REQUEST, 0, 0 }, \
      YS_INVALID_HANDLE, YS_INVALID_HANDLE, YS_INVALID_HANDLE, 0, 0, 0, 0, 0 }

typedef uint32_t ys_frame_state;
enum {
    YS_FRAME_QUEUED = 1,
    YS_FRAME_SUBMITTED = 2,
    YS_FRAME_COMPLETED = 3,
    YS_FRAME_FAILED = 4,
    YS_FRAME_CANCELLED = 5,
    YS_FRAME_OUTPUT_UNAVAILABLE = 6
};

typedef struct ys_frame_status {
    ys_struct_header header;
    ys_frame_state state;
    ys_result result;
    ys_diagnostics_id diagnostics_id;
    uint64_t user_tag;
    uint64_t submit_sequence;
} ys_frame_status;

typedef uint32_t ys_frame_report_kind;
enum {
    YS_FRAME_REPORT_WORKLOAD_JSON = 1,
    YS_FRAME_REPORT_TIMINGS_JSON = 2,
    YS_FRAME_REPORT_METRICS_JSON = 3,
    YS_FRAME_REPORT_RESOURCES_JSON = 4,
    YS_FRAME_REPORT_PROVENANCE_JSON = 5
};

enum {
    YS_REPORT_CAPTURE_BASIC_PROVENANCE = 1u << 0,
    YS_REPORT_CAPTURE_TIMINGS = 1u << 1,
    YS_REPORT_CAPTURE_METRICS = 1u << 2,
    YS_REPORT_CAPTURE_RESOURCES = 1u << 3,
    YS_REPORT_CAPTURE_WORKLOAD = 1u << 4
};

#define YS_FRAME_STATUS_INIT \
    { { (uint32_t)sizeof(ys_frame_status), YS_ABI_VERSION_1, YS_STRUCTURE_FRAME_STATUS, 0, 0 }, \
      YS_FRAME_QUEUED, YS_STATUS_NOT_READY, 0, 0, 0 }

typedef struct ys_mapped_readback {
    ys_struct_header header;
    const void* data;
    uint64_t size;
    uint64_t row_pitch;
    uint32_t width;
    uint32_t height;
    ys_pixel_format format;
    ys_color_space color_space;
} ys_mapped_readback;

#define YS_MAPPED_READBACK_INIT \
    { { (uint32_t)sizeof(ys_mapped_readback), YS_ABI_VERSION_1, YS_STRUCTURE_MAPPED_READBACK, 0, 0 }, \
      0, 0, 0, 0, 0, 0, 0 }

typedef struct ys_diagnostic_info {
    ys_struct_header header;
    ys_diagnostics_id id;
    ys_result code;
    ys_error_severity severity;
    ys_subsystem subsystem;
    uint32_t reserved;
    uint64_t related_handle;
    uint64_t submit_sequence;
    uint64_t json_required_size;
} ys_diagnostic_info;

#define YS_DIAGNOSTIC_INFO_INIT \
    { { (uint32_t)sizeof(ys_diagnostic_info), YS_ABI_VERSION_1, YS_STRUCTURE_DIAGNOSTIC_INFO, 0, 0 }, \
      0, 0, 0, 0, 0, 0, 0, 0 }

/* 无需创建 Runtime 的版本、设备与 surface 能力查询。 */
YS_API ys_result YS_CALL ys_query_abi(
    uint32_t out_info_size,
    ys_abi_info* out_info,
    ys_error_info* error);

YS_API ys_result YS_CALL ys_device_enumerate(
    ys_backend_kind backend,
    uint32_t device_index,
    ys_device_info* out_info,
    ys_error_info* error);

YS_API ys_result YS_CALL ys_output_capabilities_query(
    ys_backend_kind backend,
    uint64_t device_id,
    const ys_output_desc* requested_output,
    ys_output_capabilities* out_capabilities,
    ys_error_info* error);

/* Runtime 根对象。 */
YS_API ys_result YS_CALL ys_runtime_create(
    const ys_runtime_desc* desc,
    ys_runtime** out_runtime,
    ys_error_info* error);

YS_API ys_result YS_CALL ys_runtime_get_capabilities(
    ys_runtime* runtime,
    ys_runtime_capabilities* out_capabilities,
    ys_error_info* error);

YS_API ys_result YS_CALL ys_runtime_poll(
    ys_runtime* runtime,
    const ys_poll_desc* desc,
    ys_poll_result* out_result,
    ys_error_info* error);

YS_API ys_result YS_CALL ys_runtime_get_state(
    ys_runtime* runtime,
    ys_runtime_state* out_state,
    ys_error_info* error);

YS_API ys_result YS_CALL ys_runtime_shutdown(
    ys_runtime* runtime,
    ys_shutdown_mode mode,
    ys_error_info* error);

YS_API ys_result YS_CALL ys_runtime_destroy(
    ys_runtime* runtime,
    ys_error_info* error);

/* 离线 Render Asset Package；加载发生在稳定帧之外。 */
YS_API ys_result YS_CALL ys_asset_package_load(
    ys_runtime* runtime,
    const ys_asset_package_desc* desc,
    ys_asset_package* out_package,
    ys_error_info* error);

YS_API ys_result YS_CALL ys_asset_package_query(
    ys_runtime* runtime,
    ys_asset_package package,
    ys_asset_package_state* out_state,
    ys_diagnostics_id* out_diagnostics_id,
    ys_error_info* error);

YS_API ys_result YS_CALL ys_asset_package_release(
    ys_runtime* runtime,
    ys_asset_package package,
    ys_error_info* error);

/* Host 配置 Feature 集合，不暴露内部 Workload IR。 */
YS_API ys_result YS_CALL ys_feature_profile_create(
    ys_runtime* runtime,
    const ys_feature_profile_desc* desc,
    ys_feature_profile* out_profile,
    ys_error_info* error);

YS_API ys_result YS_CALL ys_feature_profile_update(
    ys_runtime* runtime,
    ys_feature_profile profile,
    const ys_feature_profile_desc* desc,
    ys_error_info* error);

YS_API ys_result YS_CALL ys_feature_profile_destroy(
    ys_runtime* runtime,
    ys_feature_profile profile,
    ys_error_info* error);

/* SceneTransaction 与不可变 SceneVersion。 */
YS_API ys_result YS_CALL ys_scene_transaction_begin(
    ys_runtime* runtime,
    ys_scene_version base_version,
    ys_scene_transaction* out_transaction,
    ys_error_info* error);

YS_API ys_result YS_CALL ys_scene_transaction_write(
    ys_runtime* runtime,
    ys_scene_transaction transaction,
    const void* command_records,
    uint64_t command_records_size,
    ys_error_info* error);

YS_API ys_result YS_CALL ys_scene_transaction_abort(
    ys_runtime* runtime,
    ys_scene_transaction transaction,
    ys_error_info* error);

YS_API ys_result YS_CALL ys_scene_commit(
    ys_runtime* runtime,
    ys_scene_transaction transaction,
    const ys_scene_commit_desc* desc,
    ys_scene_version* out_version,
    ys_error_info* error);

/*
 * 返回 YS_OK 时消费 transaction，并返回非零 SceneVersion。
 * 返回非 YS_OK 时 transaction 保持有效且内容不变，out_version 置为无效值。
 */

YS_API ys_result YS_CALL ys_scene_version_release(
    ys_runtime* runtime,
    ys_scene_version version,
    ys_error_info* error);

/* SceneVersion release 可从任意 Host 线程调用；只释放 Host 引用。 */

/* View 在 render 接受请求时捕获 revision。 */
YS_API ys_result YS_CALL ys_view_create(
    ys_runtime* runtime,
    const ys_view_desc* desc,
    ys_view* out_view,
    ys_error_info* error);

YS_API ys_result YS_CALL ys_view_update(
    ys_runtime* runtime,
    ys_view view,
    const ys_view_desc* desc,
    ys_error_info* error);

YS_API ys_result YS_CALL ys_view_destroy(
    ys_runtime* runtime,
    ys_view view,
    ys_error_info* error);

/* Output 与 Host 分离；平台信息只允许出现在版本化 next 扩展中。 */
YS_API ys_result YS_CALL ys_output_create(
    ys_runtime* runtime,
    const ys_output_desc* desc,
    ys_output* out_output,
    ys_error_info* error);

YS_API ys_result YS_CALL ys_output_update(
    ys_runtime* runtime,
    ys_output output,
    const ys_output_desc* desc,
    ys_error_info* error);

YS_API ys_result YS_CALL ys_output_get_info(
    ys_runtime* runtime,
    ys_output output,
    ys_output_info* out_info,
    ys_error_info* error);

YS_API ys_result YS_CALL ys_output_destroy(
    ys_runtime* runtime,
    ys_output output,
    ys_error_info* error);

/* Frame 调用非阻塞；query 只观察，不推进状态。 */
YS_API ys_result YS_CALL ys_render(
    ys_runtime* runtime,
    const ys_frame_request* request,
    ys_frame_ticket* out_ticket,
    ys_error_info* error);

YS_API ys_result YS_CALL ys_frame_query(
    ys_runtime* runtime,
    ys_frame_ticket ticket,
    ys_frame_status* out_status,
    ys_error_info* error);

YS_API ys_result YS_CALL ys_frame_release(
    ys_runtime* runtime,
    ys_frame_ticket ticket,
    ys_error_info* error);

/* FrameTicket release 可从任意 Host 线程调用；不会取消已接受工作。 */

/*
 * 映射指定已完成帧的 Offscreen Output。返回指针只读且由 Runtime 拥有；
 * 每个 ticket 同时只允许一个映射，必须由同一 Host 线程解除映射。
 */
YS_API ys_result YS_CALL ys_frame_map_readback(
    ys_runtime* runtime,
    ys_frame_ticket ticket,
    ys_mapped_readback* out_readback,
    ys_error_info* error);

YS_API ys_result YS_CALL ys_frame_unmap_readback(
    ys_runtime* runtime,
    ys_frame_ticket ticket,
    ys_error_info* error);

/*
 * 复制指定帧的只读证据报告。destination 可为 NULL 以查询 required size；
 * 报告至少保留到 FrameTicket release，不允许 Host 由此修改 Workload。
 */
YS_API ys_result YS_CALL ys_frame_report_copy_json(
    ys_runtime* runtime,
    ys_frame_ticket ticket,
    ys_frame_report_kind report_kind,
    char* destination,
    uint64_t destination_size,
    uint64_t* out_required_size,
    ys_error_info* error);

/* 诊断为结构化数据；完整内容复制到 Host 提供的 UTF-8 JSON 缓冲。 */
YS_API ys_result YS_CALL ys_diagnostic_query(
    ys_runtime* runtime,
    ys_diagnostics_id id,
    ys_diagnostic_info* out_info,
    ys_error_info* error);

YS_API ys_result YS_CALL ys_diagnostic_copy_json(
    ys_runtime* runtime,
    ys_diagnostics_id id,
    char* destination,
    uint64_t destination_size,
    uint64_t* out_required_size,
    ys_error_info* error);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* YOG_SOTHOTH_RUNTIME_HOST_C_ABI_H */
