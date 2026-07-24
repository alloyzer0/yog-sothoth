# Issue 跟踪器：本地 Markdown

本仓库的 issue 和规格文档（也称为 PRD）以 Markdown 文件形式存放在 `.scratch/` 中。

## 约定

- 每个功能使用一个独立目录：`.scratch/<feature-slug>/`
- 规格文档位于 `.scratch/<feature-slug>/spec.md`
- 每个实现 issue 使用一个独立文件，路径为 `.scratch/<feature-slug>/issues/<NN>-<slug>.md`；编号从 `01` 开始——不得把所有 ticket 合并到一个文件中
- 分类状态记录在每个 issue 文件顶部附近的 `Status:` 行中（角色字符串详见 `triage-labels.md`）
- 评论和对话历史追加到文件底部的 `## Comments` 标题下

## 当技能要求“发布到 issue 跟踪器”时

在 `.scratch/<feature-slug>/` 下创建新文件；如目录不存在，则一并创建。

## 当技能要求“获取相关 ticket”时

读取所引用路径中的文件。用户通常会直接提供文件路径或 issue 编号。

## Wayfinding 操作

供 `/wayfinder` 使用。**地图（map）**由一个文件构成，每个 ticket 对应一个独立的**子文件（child）**。

- **地图（Map）**：`.scratch/<effort>/map.md`——保存 Notes、Decisions-so-far 和 Fog 正文。
- **子 ticket（Child ticket）**：`.scratch/<effort>/issues/NN-<slug>.md`，编号从 `01` 开始，正文中记录问题。`Type:` 行记录 ticket 类型（`research`/`prototype`/`grilling`/`task`）；`Status:` 行记录状态（`claimed`/`resolved`）。
- **阻塞（Blocking）**：在文件顶部附近使用 `Blocked by: NN, NN` 行记录。当其中列出的每个文件状态均为 `resolved` 时，该 ticket 解除阻塞。
- **前沿（Frontier）**：扫描 `.scratch/<effort>/issues/`，查找处于开放、未阻塞且未认领状态的文件；编号最小者优先。
- **认领（Claim）**：开始任何工作前，将 `Status:` 设置为 `claimed` 并保存。
- **解决（Resolve）**：把答案追加到 `## Answer` 标题下，将 `Status:` 设置为 `resolved`，然后在 `map.md` 的 Decisions-so-far 中追加上下文指针（摘要和链接）。
