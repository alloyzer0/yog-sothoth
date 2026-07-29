## Agent skills

### Issue tracker

Issue 和规格文档以本地 Markdown 文件形式存放在 `.scratch/<feature-slug>/` 下。详见 `docs/agents/issue-tracker.md`。

### Triage labels

使用五个默认的标准分类标签。详见 `docs/agents/triage-labels.md`。

### Domain docs

本仓库采用 single-context（单上下文）领域文档布局。详见 `docs/agents/domain.md`。

### Code style

C/C++ 代码以根目录 `.clang-format` 为格式化真源。修改后运行 `format-check`，命名与例外规则详见 `docs/agents/code-style.md`。

### Learning workflow

本仓库采用学习优先的协作方式；除非用户明确要求直接实现，否则核心设计、算法和主要实现由用户完成。详见 `docs/agents/learning-workflow.md`。
