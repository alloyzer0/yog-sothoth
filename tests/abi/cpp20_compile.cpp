#include <type_traits>

#include <yog_sothoth/runtime.h>

static_assert(std::is_standard_layout_v<ys_struct_header>);
static_assert(std::is_trivially_copyable_v<ys_frame_status>);

int main()
{
    auto runtime_desc = ys_runtime_desc YS_RUNTIME_DESC_INIT;
    auto output_desc = ys_output_desc YS_OUTPUT_DESC_INIT;

    (void)runtime_desc;
    (void)output_desc;
    return 0;
}
