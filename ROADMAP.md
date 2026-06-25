# CLEngine2D 开发路线图

## 概述

整个引擎分为 **7 个阶段**，每个阶段都有明确的目标、交付物和关键技术点。建议按顺序推进，每个阶段完成后再进入下一个。

---

## 阶段一：基础骨架

**目标**：跑通第一个窗口，建立项目的工程结构和构建系统。

### 任务

- [x] 初始化 CMake 项目，配置 C++11 标准
- [x] 集成 GLFW，创建窗口并处理窗口事件（关闭、缩放、聚焦）
- [x] 创建游戏循环骨架（`Init → Update → Render → Shutdown`）
- [x] 集成 Glad（OpenGL 3.3+ Core Profile）
- [x] 验证 OpenGL 上下文成功加载（打印版本号、驱动信息）
- [x] 添加简单的 `Application` 类封装生命周期
- [x] 添加 `Logger` 工具类（控制台带颜色输出，支持 LogLevel）

### 关键技术点

- CMake `FetchContent` 或 `add_subdirectory` 管理第三方库
- `glfwWindowHint` 配置 OpenGL 版本
- 分离 `OnUpdate(dt)` 和 `OnRender()` 两个阶段
- 时间步长：固定步长 vs 可变步长，先使用可变步长

### 交付物

```
src/engine/
├── Public/                # 公开 API（外部可见）
│   ├── Application.h      # 引擎入口类
│   ├── Logger.h           # 日志系统
│   └── Platform/
│       └── Window.h       # GLFW 窗口封装
├── Private/               # 内部实现
│   ├── Application.cpp
│   ├── Logger.cpp
│   └── Platform/
│       └── Window.cpp
└── CMakeLists.txt
```

运行后显示一个空白窗口，控制台输出引擎版本和 OpenGL 信息。

---

## 阶段二：渲染核心

**目标**：封装 OpenGL，能绘制带纹理的精灵（Sprite）。

### 任务

- [x] 编写着色器类 `Shader`（加载 vertex/fragment shader，uniform 设置）
- [x] 编写 `VertexBuffer` / `IndexBuffer` / `VertexArray` 对象封装
- [x] 编写 `Texture` 类（stb_image 加载图片，生成 OpenGL 纹理）
- [x] 实现 `Renderer` 类（封装 `glDrawElements`，支持提交 sprite）
- [x] 实现 `RenderCommand`（设置 clear color、视口等简单命令）
- [x] 实现 `OrthographicCamera`（投影矩阵、变换矩阵）
- [x] 实现精灵批处理（Sprite Batch）：单次 draw call 渲染多个精灵
- [x] 实现纹理图集（Texture Atlas）基础支持

### 关键技术点

- RAII 管理 OpenGL 资源（构造函数创建，析构函数 glDelete）
- Shader 文件用外部 `.glsl` 文件存储，运行时加载
- 精灵批处理的 4 顶点 + 2 三角形 + UV 坐标布局
- 使用 glm 作为数学库（建议先自己写一个极简数学库理解原理，再切到 glm）
- `OrthographicCamera` 的 View-Projection 矩阵推导

### 交付物

```
src/engine/Public/Render/
├── Shader.h
├── VertexBuffer.h
├── IndexBuffer.h
├── VertexArray.h
├── Texture.h
├── Renderer.h
├── RenderCommand.h
└── OrthographicCamera.h
src/engine/Private/Render/
├── Shader.cpp
├── VertexBuffer.cpp
├── IndexBuffer.cpp
├── VertexArray.cpp
├── Texture.cpp
├── Renderer.cpp
├── RenderCommand.cpp
└── OrthographicCamera.cpp
```

能加载 PNG 纹理，在窗口中绘制带纹理的矩形并支持平移缩放。

---

## 阶段三：数学库（可选但推荐）

**目标**：理解游戏数学原理，建立自己的数学库。

### 任务

- [ ] `Vec2` / `Vec3` / `Vec4`（加减乘除、点积、叉积、归一化）
- [ ] `Mat4`（矩阵乘法、转置、逆矩阵）
- [ ] 变换函数（`Translate`、`Rotate`、`Scale`）
- [ ] `Ortho` 投影矩阵
- [ ] 测试：验证结果与 glm 一致

### 关键技术点

- 列主序存储（匹配 OpenGL）
- SIMD 优化留到后期
- 理解齐次坐标（为什么需要 `w` 分量）

### 交付物

```
src/engine/Public/Math/
├── Vec2.h / Vec3.h / Vec4.h
├── Mat4.h
└── Transform.h
```

---

## 阶段四：资产与场景管理

**目标**：能加载多种资源并组织游戏对象。

### 任务

- [ ] `TextureManager`（缓存已加载纹理，防止重复加载）
- [ ] `ShaderManager`（同上）
- [ ] 简单的 `Sprite` 组件（位置、旋转、缩放、纹理）
- [ ] `Scene` 类（管理一组 Sprite 的更新和渲染）
- [ ] 支持场景切换（`PushScene` / `PopScene`）
- [ ] 添加 `Timer` 工具（帧率统计、性能打点）

### 关键技术点

- 单例 vs 依赖注入（建议先用单例，后期再重构）
- unordered_map 做资源缓存
- 场景栈管理（类似游戏的状态栈）

### 交付物

```
src/engine/Public/
├── AssetManager.h
├── Scene.h
├── Timer.h
└── Components/
    └── SpriteComponent.h
src/engine/Private/
├── AssetManager.cpp
├── Scene.cpp
├── Timer.cpp
└── Components/
    └── SpriteComponent.cpp
```

---

## 阶段五：输入与音频

**目标**：键盘鼠标手柄输入 + 音效播放。

### 任务

- [ ] `Input` 单例（查询按键状态：`IsKeyDown`、`IsMouseButtonPressed`）
- [ ] 键盘、鼠标、手柄回调封装
- [ ] 集成 miniaudio（单头文件，无依赖）
- [ ] `AudioSource` / `Sound` 封装（加载 wav，播放，音量控制）
- [ ] 简单的 2D 音频衰减（距离衰减）

### 关键技术点

- GLFW 回调注册 vs 轮询
- 键码映射抽象（不直接暴露 GLFW 键码）
- miniaudio 的设备初始化和资源管理

### 交付物

```
src/engine/Public/
├── Input.h
├── InputCodes.h          # 自定义键码枚举
└── Audio/
    ├── Sound.h
    └── AudioEngine.h
src/engine/Private/
├── Input.cpp
└── Audio/
    ├── Sound.cpp
    └── AudioEngine.cpp
```

---

## 阶段六：完整 2D 游戏能力

**目标**：具备构建一个小游戏的全部能力。

### 任务

- [ ] `TextRenderer`（使用 FreeType 渲染 TrueType 字体）
- [ ] 粒子系统（`ParticleEmitter`，支持自定义生命周期/颜色/速度）
- [ ] 2D 碰撞检测（AABB、Circle，碰撞响应弹开）
- [ ] 图层渲染（Layer / Z-order）
- [ ] Camera 控制器（跟随目标、平滑插值）
- [ ] 动画系统（SpriteSheet 帧动画）
- [ ] `TiledMap` 支持（解析 .tmx 文件或自定义格式）

### 交付物

```
src/engine/Public/
├── TextRenderer.h
├── ParticleSystem.h
├── Physics2D.h
├── Animator.h
└── TiledMap.h
src/engine/Private/
├── TextRenderer.cpp
├── ParticleSystem.cpp
├── Physics2D.cpp
├── Animator.cpp
    └── TiledMap.cpp
```

---

## 阶段七：Python 脚本集成

**目标**：C++ 引擎核心功能齐备后，接入 pybind11，将关键 API 暴露给 Python，实现 Python 驱动的游戏业务层。性能敏感的算法模块（物理、寻路、粒子更新等）继续保留 C++ 实现，Python 仅调用入口函数。

### 任务

- [ ] 集成 pybind11（CMake FetchContent）
- [ ] 暴露核心 C++ API：
  - Application（生命周期、帧回调注册）
  - Renderer（提交 Sprite、清屏）
  - Input（按键/鼠标查询）
  - Scene / SpriteComponent（场景管理与游戏对象操作）
  - Timer（帧率、delta time）
  - Math（Vec3、Mat4 基础运算）
- [ ] 实现嵌入模式：C++ 主循环每帧回调 Python 的 `on_update(dt)` / `on_render()`
- [ ] 创建 `scripts/` 目录结构
- [ ] 编写 `scripts/main.py` 入口脚本，展示 Python 中创建实体、处理输入、切换场景
- [ ] 新增 `src/engine/PythonBind/` 绑定模块目录
- [ ] 补充构建说明：`pip install -r scripts/requirements.txt`

### 关键技术点

- pybind11 的 `PYBIND11_MODULE` 宏和 `.def()` 链式绑定
- 嵌入模式：C++ 为骨架跑主循环，每帧回调 Python 执行业务逻辑
- 跨语言对象生命周期管理（`py::keep_alive`，确保 C++ 对象在 Python 侧存活）
- 大计算量模块（碰撞检测、A* 寻路、粒子更新）保留 C++ 完整实现，Python 只调顶层入口
- 每帧 Python → C++ 跨语言调用控制在几百次以内，开销可忽略（单次 ≈ 100-300ns）

### 交付物

```
src/engine/
├── PythonBind/                 # pybind11 绑定代码（新增）
│   ├── PyEngine.cpp            # 模块入口
│   ├── BindApp.cpp/h
│   ├── BindRenderer.cpp/h
│   ├── BindInput.cpp/h
│   ├── BindScene.cpp/h
│   └── BindMath.cpp/h
scripts/                        # Python 业务脚本（新增）
├── main.py                     # 入口
├── requirements.txt
├── game/
│   ├── player.py
│   └── enemy.py
├── scenes/
│   ├── menu_scene.py
│   └── game_scene.py
└── components/
    ├── sprite_animator.py
    └── ai_controller.py
```

---

## 架构总览

```
src/
├── engine/
│   ├── Public/                   # 公开 API 头文件
│   │   ├── Platform/             # 公开的平台抽象
│   │   ├── Render/               # 渲染子系统
│   │   ├── Audio/                # 音频子系统
│   │   ├── Math/                 # 数学库
│   │   └── Components/           # 组件定义
│   ├── Private/                  # 内部实现（.cpp + 内部头文件）
│   │   ├── Platform/
│   │   ├── Render/
│   │   ├── Audio/
│   │   ├── Math/
│   │   └── Components/
│   ├── PythonBind/               # pybind11 绑定层（阶段七新增）
│   │   ├── PyEngine.cpp
│   │   ├── BindApp.cpp/h
│   │   ├── BindRenderer.cpp/h
│   │   ├── BindInput.cpp/h
│   │   ├── BindScene.cpp/h
│   │   └── BindMath.cpp/h
│   └── CMakeLists.txt
├── scripts/                      # Python 业务脚本（阶段七新增）
│   ├── main.py                   # Python 入口
│   ├── requirements.txt
│   ├── game/
│   ├── scenes/
│   └── components/
├── sandbox/                      # 测试/示例项目
│   ├── CMakeLists.txt
│   └── main.cpp
└── third_party/                  # 第三方库
    ├── glad/
    ├── glfw/
    └── stb_image/
```

---

## 原则与约定

1. **理解优先于封装**：每个第三方库引入时，先写一个最小 demo 理解它在做什么，再封装
2. **不做过度抽象**：前期接口尽量简单，必要时再重构
3. **每步可运行**：每个任务完成后都能编译运行看到效果，避免长时间不可运行
4. **C++11 为主**：默认使用 C++11 特性，若需要使用 C++11 以上的特性（如 `std::format`、`std::optional`、`std::span` 等），需提前与开发者确认
5. **无异常**：使用返回值和错误码处理错误
