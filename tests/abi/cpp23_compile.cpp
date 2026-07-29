#include <yog_sothoth/runtime.h>

#if defined(_MSVC_LANG)
#  define YS_HOST_CPLUSPLUS _MSVC_LANG
#else
#  define YS_HOST_CPLUSPLUS __cplusplus
#endif

static_assert(
    YS_HOST_CPLUSPLUS > 202002L,
    "This Host compatibility probe must compile in standard C++23 mode.");

int main()
{
    const ys_runtime_desc runtime_desc = YS_RUNTIME_DESC_INIT;
    const ys_output_desc output_desc = YS_OUTPUT_DESC_INIT;

    (void)runtime_desc;
    (void)output_desc;
    return 0;
}
