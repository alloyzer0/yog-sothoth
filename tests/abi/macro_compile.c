#include <yog_sothoth/runtime.h>

/* 只验证声明在当前宏模式下可形成函数指针类型，不链接实现。 */
/* clang-format off: 保留调用约定函数指针声明的可读分组。 */
typedef ys_result(
    YS_CALL* ys_runtime_create_signature)(const ys_runtime_desc*, ys_runtime**, ys_error_info*);
/* clang-format on */

static ys_runtime_create_signature signature_check = &ys_runtime_create;

int ys_macro_compile_probe(void)
{
    return signature_check != 0;
}
