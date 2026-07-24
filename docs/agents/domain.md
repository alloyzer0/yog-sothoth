# 领域文档

本文规定工程技能在探索代码库时应如何读取本仓库的领域文档。

## 开始探索前，请读取以下内容

- 仓库根目录下的 **`CONTEXT.md`**；或者
- 如果根目录存在 **`CONTEXT-MAP.md`**，则读取该文件。它会指向每个上下文各自的 `CONTEXT.md`，请读取与当前主题相关的文件。
- **`docs/adr/`**——读取与即将处理区域相关的 ADR。在 multi-context 仓库中，还需检查 `src/<context>/docs/adr/` 中限定于该上下文的决策。

如果上述文件不存在，**直接继续，不作提示**。不要报告文件缺失，也不要预先建议创建它们。`/domain-modeling` 技能（可通过 `/grill-with-docs` 和 `/improve-codebase-architecture` 使用）会在术语或决策真正明确后按需创建这些文件。

## 文件结构

Single-context 仓库（适用于大多数仓库）：

```text
/
├── CONTEXT.md
├── docs/adr/
│   ├── 0001-event-sourced-orders.md
│   └── 0002-postgres-for-write-model.md
└── src/
```

Multi-context 仓库（根目录存在 `CONTEXT-MAP.md`）：

```text
/
├── CONTEXT-MAP.md
├── docs/adr/                          ← 系统级决策
└── src/
    ├── ordering/
    │   ├── CONTEXT.md
    │   └── docs/adr/                  ← 特定上下文的决策
    └── billing/
        ├── CONTEXT.md
        └── docs/adr/
```

## 使用术语表中的词汇

当输出内容需要命名某个领域概念时（例如 issue 标题、重构提案、假设或测试名称），请使用 `CONTEXT.md` 中定义的术语，不要改用术语表明确要求避免的同义词。

如果术语表中尚未包含所需概念，这是一个需要关注的信号：可能是你正在创造项目并未使用的语言（请重新考虑），也可能确实存在领域词汇缺口（记录下来，交由 `/domain-modeling` 处理）。

## 标记与 ADR 的冲突

如果输出内容与现有 ADR 冲突，请明确指出，不要静默覆盖：

> _与 ADR-0007（事件溯源订单）冲突——但值得重新讨论，因为……_
