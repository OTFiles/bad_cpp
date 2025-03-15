# bad_cpp

这是一个将视频转换为 ASCII 字符并在终端中实时播放的 C++ 程序。

## 功能特点
- **自适应终端尺寸**：根据终端窗口大小动态调整输出分辨率。
- **精确帧率控制**：使用高精度计时器确保视频播放速度与原视频一致。
- **轻量级**：仅依赖 FFmpeg 库，无需 OpenCV 或其他重型库。

## 使用方法

### 1. 安装依赖

在 Termux 中运行以下命令安装所需依赖：

```bash
pkg update && pkg upgrade
pkg install ffmpeg clang
```

### 2. 编译程序

将代码保存为 `main.cpp`，然后使用以下命令编译：

```bash
g++ -o ascii_video main.cpp -lavformat -lavcodec -lswscale -lavutil -lm
```

### 3. 运行程序

将视频文件命名为 `in.mp4`，并确保它与程序在同一目录下。运行以下命令开始播放：

```bash
./ascii_video
```

按 `Ctrl+C` 退出程序。

## 代码结构

- **`main.cpp`**：主程序文件，包含视频解码、ASCII 转换和终端渲染逻辑。
- **依赖库**：
  - `libavcodec`：视频解码。
  - `libavformat`：视频文件解析。
  - `libswscale`：图像缩放和格式转换。
  - `libavutil`：工具函数。

## 参数说明

- **视频文件**：默认读取当前目录下的 `in.mp4` 文件。
- **终端尺寸**：自动检测终端窗口大小并调整输出分辨率。
- **帧率控制**：根据视频的帧率动态调整播放速度。

## 示例

### 测试视频生成

可以使用以下命令生成一个测试视频：

```bash
ffmpeg -f lavfi -i testsrc=duration=10:size=640x480:rate=30 test.mp4
```

然后将 `test.mp4` 重命名为 `in.mp4` 并运行程序。

## 常见问题

### 1. 播放速度过快

确保编译时启用了高精度计时器（`-std=c++11`），并检查视频的帧率是否正确解析。

### 2. 终端显示错乱

- 确保终端支持 ANSI 转义码。
- 调整终端窗口大小以获得最佳显示效果。

### 3. 编译错误

- 确保安装了所有依赖库（`ffmpeg` 和 `clang`）。
- 如果出现链接错误，尝试调整库的顺序：
  ```bash
  g++ -std=c++11 -o ascii_video main.cpp -lavformat -lavcodec -lswscale -lavutil -lm
  ```

### 4. 视频无法播放

- 检查视频文件路径和格式。
- 使用 `ffmpeg -i in.mp4` 确认视频文件是否有效。

## 性能优化

- **降低分辨率**：使用较小的终端窗口以减少渲染负载。
- **简化 ASCII 字符集**：减少字符数量以提高渲染速度。
- **丢帧机制**：在性能不足时跳过部分帧以保持同步。

## 未来改进

- **音频支持**：集成音频播放以实现完整的视频播放体验。
- **颜色支持**：支持彩色 ASCII 渲染。
- **交互控制**：添加暂停、快进、快退等功能。

## 许可证

本项目基于 **GNU General Public License v3.0** 许可证开源。  
详情请参阅 [LICENSE](LICENSE) 文件。

## 贡献

欢迎提交 Issue 或 Pull Request 以改进本项目！

## 作者

- **OTFiles** - 项目开发者
- **GitHub**: [OTFiles](https://github.com/OTFiles)

## 致谢

- 感谢 FFmpeg 团队提供的强大多媒体库。
- 感谢 Termux 社区提供的 Android 终端环境支持。

## 反馈

如有任何问题或建议，请通过 [Issues](https://github.com/OTFiles/bad_cpp/issues) 反馈。
