#if defined(_MSVC_LANG)
#  define YS_INTERNAL_CPLUSPLUS _MSVC_LANG
#else
#  define YS_INTERNAL_CPLUSPLUS __cplusplus
#endif

static_assert(
    YS_INTERNAL_CPLUSPLUS > 202002L,
    "Yog-Sothoth internal implementation requires standard C++23 mode.");

int main() {
    return 0;
}
