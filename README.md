# TinyEngine - 从零开始的 2D 游戏引擎

## 项目背景

这是一个用于学习游戏引擎底层原理的个人项目。目标是从零构建一个轻量级 2D 游戏引擎，不依赖现有游戏引擎框架，深入理解计算机图形学、实时渲染、事件系统、资源管理等底层知识。

## 为什么是 2D？

2D 引擎在架构上与 3D 引擎共享核心设计——游戏循环、ECS/组件系统、渲染管线、输入管理、音频系统等，但绕开了 3D 引擎特有的复杂度（透视投影、矩阵变换、光照模型、深度测试等），让我能更快触及引擎架构的本质。

## 技术栈

- **语言**: C++23（按 C++11 风格编写）
- **图形 API**: 待定（OpenGL 3.3+ / DirectX 11 / Vulkan）
- **窗口/上下文**: GLFW
- **构建系统**: CMake
- **音频**: OpenAL Soft / miniaudio
- **脚本**: Python 3.14（pybind11 绑定 C++ API）

## 学习路线

1. 窗口管理与事件循环
2. 图形 API 封装与渲染管线
3. 纹理加载与精灵渲染
4. 数学库（向量、矩阵、变换）
5. ECS 架构设计
6. 资源管理器（纹理、音频、Shader）
7. 输入系统
8. 音频系统
9. 2D 物理（AABB、碰撞检测）
10. 粒子系统
11. 场景/状态管理
12. 性能分析与调试工具

## 构建

```bash
cmake -B build -S .
cmake --build build
```

可执行文件在 `build/src/sandbox/Release/sandbox.exe`（或 `Debug/`）。

### Python 业务脚本

Python 业务层依赖安装在 `scripts/` 目录下：

```bash
pip install -r scripts/requirements.txt
```
