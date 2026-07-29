# ADR-0006：Headless-first Vulkan 启动与 Evidence Artifact 验证

Status: accepted

Yog-Sothoth 的首个真实 GPU 路径使用 Linux x86-64 上的 Vulkan Offscreen Output，不以 Viewer、Swapchain 或图形桌面会话作为前置条件。Frame 通过 readback 形成包含图像、配置、设备信息、timings、metrics 和 provenance 的 Evidence Artifact，供人工检查、像素断言、图像回归和性能实验使用。

该选择优先隔离并学习 Vulkan device、queue、memory、command、同步和资源退休，避免窗口系统复杂性阻塞第一像素，同时适配远程开发和 CI。Win32 Viewer/Swapchain 保留为后续第二个 Host/Output Adapter，用于验证平台边界，而不是 Runtime Foundation 的完成条件。
