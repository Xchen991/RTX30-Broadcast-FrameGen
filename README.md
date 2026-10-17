# NVIDIA Broadcast 视频插帧补丁（RTX 30 系列）

**中文** · [English](README.en.md)

在 NVIDIA GeForce RTX 30 系列（Ampere / SM86）显卡上解锁并开启 NVIDIA Broadcast 的「视频插帧（Video Frame Generation）」功能。

单文件代理 DLL（`version.dll`），体积仅约 11 KB，不修改任何官方原始文件，零额外依赖。

## 运行要求

- **操作系统**：Windows 11 64 位
- **显卡支持**：NVIDIA GeForce RTX 30 系列（RTX 3050 / 3060 / 3070 / 3080 / 3090 等）
- **软件要求**：已安装最新版 NVIDIA Broadcast（带相机效果组件）
- **显卡驱动**：617.42 或更新版本驱动

## 安装与使用

1. **退出软件**：右下角系统托盘退出 NVIDIA Broadcast（若提示占用，请在任务管理器中结束对应进程）。
2. **复制补丁**：下载 Releases 页面提供的 `version.dll`，直接复制到软件安装目录下：
   C:\Program Files\NVIDIA Corporation\NVIDIA Broadcast\
   （注：该目录下默认没有同名文件，直接放进去即可，无需替换任何文件。）
3. **开启插帧**：打开 NVIDIA Broadcast，在“相机”设置中找到「视频插帧」开关，此时已解除置灰限制，开启即可输出流畅 60 帧。
4. **卸载方法**：退出软件后，直接删除刚才放进去的 `version.dll` 即可彻底还原。

## 常见问题

- **开关依然是灰色**：请检查显卡驱动是否过旧（建议升级至最新版），并确认 `version.dll` 是放在安装根目录下（与 `NVIDIA Broadcast.exe` 同级）。
- **文件复制提示被占用**：在任务管理器中彻底结束 `NVIDIA Broadcast` 相关的所有后台进程后再复制。

## 许可证

本项目源码基于 [MIT 许可证](LICENSE) 发布。