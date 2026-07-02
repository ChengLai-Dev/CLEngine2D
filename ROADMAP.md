# CLEngine2D 开发路线图

## 概述

整个引擎分为 **9 个阶段**，每个阶段都有明确的目标、交付物和关键技术点。建议按顺序推进，每个阶段完成后再进入下一个。

## 当前完成进度总览

```
阶段一 基础骨架      ████████████████████ 100% ✅
阶段二 渲染核心      ████████████████████ 100% ✅
阶段三 数学库        ████████████████████ 100% ✅
阶段四 资产与场景    ████████████████████ 100% ✅
阶段五 输入与音频    ████████████████████ 100% ✅
阶段六 场景图系统    ████████████████████ 100% ✅
阶段七 UI 编辑器     ████████████████████ 100% ✅
阶段八 2D 游戏能力   ░░░░░░░░░░░░░░░░░░░░   0% ❌
阶段九 Python 脚本   ░░░░░░░░░░░░░░░░░░░░   0% ❌
───────────────────────────────────────────
整体项目完成度                          ~78%
```

---

## 阶段一：基础骨架

**目标**：跑通第一个窗口，建立项目的工程结构和构建系统。

### 任务

- [x] 初始化 CMake 项目，配置 C++23 标准（实际配置）
- [x] 集成 GLFW，创建窗口并处理窗口事件（关闭、缩放、聚焦）
- [x] 创建游戏循环骨架（`Init → Update → Render → Shutdown`）
- [x] 集成 Glad（OpenGL 4.5 Core Profile，实际使用版本）
- [x] 验证 OpenGL 上下文成功加载（打印版本号、驱动信息）
- [x] 添加简单的 `Application` 类封装生命周期
- [x] 添加 `Logger` 工具类（控制台带颜色输出，支持 LogLevel）
- [x] 添加 `Utils` 工具类（时间字符串格式化、调试检测）

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
- [x] 实现简单的 `Vec3` 和 `Mat4` 数学库（头文件内联，满足渲染需求）
- [ ] 实现 `Renderer` 性能统计接口、更多 `DrawQuad` 重载

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

- [x] `Vec2` / `Vec3` / `Vec4`（完整运算符集、点积、叉积、长度、归一化）
- [x] `Mat4`（矩阵乘法、转置、逆矩阵、正交投影、透视投影、平移、旋转 X/Y/Z、缩放、LookAt、TransformPoint/Vector）
- [x] 变换函数（`Translate`、`Rotate`、`Scale`）—— 位于 `Transform.h` 和 `Mat4` 静态方法
- [x] `Ortho` / `Perspective` / `LookAt` 投影与视图矩阵
- [x] 从 `Render/` 迁移至独立 `Math/` 目录
- [ ] 测试：验证结果与 glm 一致（需搭建测试框架）

### 关键技术点

- 列主序存储（匹配 OpenGL）✅ 已实现
- SIMD 优化留到后期
- 理解齐次坐标（为什么需要 `w` 分量）

### 交付物

```
src/engine/Public/Math/
├── Vec2.h       # 2D 向量
├── Vec3.h       # 3D 向量（含叉积、长度、归一化）
├── Vec4.h       # 4D 齐次坐标向量
├── Mat4.h       # 4x4 列主序矩阵（含转置、逆、投影、视图）
├── Transform.h  # Math:: 命名空间下的自由函数封装
└── Math.h       # 便捷头文件
```

---

## 阶段四：资产与场景管理

**目标**：能加载多种资源并组织游戏对象。

### 任务

- [x] `TextureManager`（缓存已加载纹理，防止重复加载）
- [x] `ShaderManager`（同上）
- [x] `Scene` 类（管理一组 Sprite 的更新和渲染）
- [x] 支持场景切换（`PushScene` / `PopScene`）
- [x] 添加 `Timer` 工具（帧率统计、性能打点）

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
    └── SpriteComponent.h           ← 已废弃，由 SceneGraph/ 替代
src/engine/Private/
├── AssetManager.cpp
├── Scene.cpp
├── Timer.cpp
└── Components/
    └── SpriteComponent.cpp         ← 已删除
```

---

## 阶段五：输入与音频

**目标**：键盘鼠标手柄输入 + 音效播放。

### 任务

- [x] `InputCodes` 自定义键码/鼠标码/手柄码枚举（与 GLFW 值对齐，不暴露 GLFW 头文件）
- [x] `RawInput` 底层轮询层（`IsKeyDown/Pressed/Released`、鼠标位置滚轮、手柄轴/按钮）
- [x] `InputAction` 输入动作（值轴 + `OnStarted/OnTriggered/OnCompleted` 回调）
- [x] `InputMappingContext` 映射上下文（将按键/鼠标绑定到 `InputAction`，支持缩放系数）
- [x] `InputSystem` 单例（管理多个 `InputMappingContext`，按优先级排序，每帧汇总分发）
- [x] 集成 miniaudio（单头文件 v0.11.25，无额外依赖）
- [x] `AudioEngine` 单例（初始化/关闭 miniaudio 引擎、加载音效、设置听者位置）
- [x] `Sound` 音效对象（`Play/Stop/Pause/Resume`、音量/循环/位置、距离衰减参数）
- [x] 2D 音频衰减（`MinDistance` / `MaxDistance` / `Rolloff`）

### 关键技术点

- GLFW 回调注册 vs 轮询（`RawInput` 使用 GLFW 回调+每帧 `glfwPollEvents`）
- 键码映射抽象（`InputCodes.h` 自定义枚举，不直接暴露 GLFW 键码）
- 输入架构模式：`RawInput` → `InputAction` / `InputMappingContext` → `InputSystem`（类似 UE 的 Enhanced Input）
- miniaudio 的设备初始化和资源管理（`ma_engine` / `ma_sound`）

### 交付物

```
src/engine/Public/
├── Input/
│   ├── InputCodes.h           # 键码/鼠标码/手柄码枚举
│   ├── RawInput.h             # 底层轮询层
│   ├── InputAction.h          # 输入动作（轴值 + 事件回调）
│   ├── InputMappingContext.h  # 映射上下文（按键→动作绑定）
│   └── InputSystem.h          # 输入系统单例
└── Audio/
    ├── Sound.h                # 音效播放控制
    └── AudioEngine.h          # 音频引擎单例
src/engine/Private/
├── Input/
│   ├── RawInput.cpp
│   ├── InputAction.cpp
│   ├── InputMappingContext.cpp
│   └── InputSystem.cpp
└── Audio/
    ├── Sound.cpp
    └── AudioEngine.cpp
```

---

## 阶段六：场景图系统

**目标**：建立层次化场景图架构，为 UI 编辑器和 3D 扩展做准备。

### 任务

- [x] `Node` 基类（场景图节点：Vec3 变换 / Euler 旋转 / 锚点 / 层级 / 可见性级联）
- [x] `Sprite` 节点（继承 Node，纹理渲染，游戏 + UI 共享）
- [x] `Scene` 重构（从 `vector<SpriteComponent>` 迁移到 `Node` 树递归遍历）
- [x] `Renderer` 支持 `Mat4` 变换绘制（`DrawQuad(Mat4, Vec2, ...)`）
- [ ] `Widget` 基类（交互、触摸、焦点，UI 编辑器基础）
- [ ] `CanvasPanel` 容器（锚点布局系统，类似 UE UMG）
- [ ] 九宫格缩放支持（`Scale9Sprite`）
- [ ] UI 编辑器序列化格式（JSON / 二进制）

### 关键技术点

- 变换层级与脏标记传播（MarkDirty 递归）
- 锚点归一化 + ContentSize 的局部变换矩阵推导
- ZOrder 排序（负值在自身前绘制，同 Cocos visit 遍历顺序）
- 不透明度级联（parentOpacity * childOpacity）
- 游戏场景与 UI 场景分离渲染（两个 pass 合成）

### 交付物

```
src/engine/Public/SceneGraph/
├── Node.h                # 场景图基类
└── Sprite.h              # 纹理精灵节点
src/engine/Private/SceneGraph/
├── Node.cpp
└── Sprite.cpp
src/engine/Public/
├── Scene.h               # 重构：持有 root Node 树
└── ...

src/engine/Private/
├── Scene.cpp             # 重构：递归 Visit 遍历
└── ...
```

---

## 阶段七：UI 编辑器

**目标**：构建可视化 UI 编辑系统，类似 Cocos Studio 或 UE UMG。

### 任务

- [x] 完整 Widget 控件库（Widget 基类、Button、Image、Label）
- [x] `Layout` 容器（VBox、HBox、Grid）
- [x] `CanvasPanel` 锚点布局容器
- [x] `Scale9Sprite` 九宫格缩放精灵
- [x] `UISystem` 触摸事件分发系统
- [x] UI 场景独立渲染 Pass（sandbox 双 Pass 演示）
- [x] 编辑器工程 `src/editor/`
- [x] CanvasView 画布视图（网格/滚轮缩放/平移/视口裁剪）
- [x] Gizmo 拖拽手柄（8 控制点 + 选择框）
- [x] PropertyPanel 属性面板
- [x] WidgetTreePanel 层级树面板
- [x] Toolbar 控件工具栏
- [x] 运行时 UI 序列化/反序列化（JSON）
- [x] UndoRedo 撤销/重做栈

### 关键技术点

- 编辑器与运行时共享同一套 Widget 类
- 序列化为 JSON / FlatBuffers（类似 Cocos .csb 格式）
- 屏幕空间与游戏世界空间转换
- DPI 缩放适配

### 交付物

```
src/engine/Public/SceneGraph/    (运行时控件库)
├── Widget.h                     Widget 基类（交互/焦点/HitTest）
├── Button.h                     交互按钮（三状态/点击回调）
├── Label.h                      文本控件
├── Image.h                      图片控件（含九宫格缩放）
├── Layout.h                     自动布局容器（VBox/HBox/Grid）
├── CanvasPanel.h                锚点布局容器（类似 UE UMG）
├── Scale9Sprite.h               九宫格缩放精灵
└── UISystem.h                   触摸事件分发系统
src/engine/Private/SceneGraph/
├── Widget.cpp
├── Button.cpp
├── Label.cpp
├── Image.cpp
├── Layout.cpp
├── CanvasPanel.cpp
├── Scale9Sprite.cpp
└── UISystem.cpp
src/editor/                      # 独立编辑器工程
├── CMakeLists.txt
├── main.cpp                     编辑器入口
├── Public/                      公开 API 头文件
│   ├── EditorApp.h              编辑器主程序（Application 子类）
│   ├── CanvasView.h             画布视图（网格/缩放/平移/裁剪）
│   ├── PropertyPanel.h          属性面板
│   ├── WidgetTreePanel.h        层级树面板
│   ├── Toolbar.h                控件工具栏
│   ├── Gizmo.h                  拖拽手柄（8 控制点 + 选择框）
│   ├── Serializer.h             JSON 序列化/反序列化
│   └── UndoRedo.h               撤销/重做栈
└── Private/                     内部实现
    ├── EditorApp.cpp
    ├── CanvasView.cpp
    ├── PropertyPanel.cpp
    ├── WidgetTreePanel.cpp
    ├── Toolbar.cpp
    ├── Gizmo.cpp
    ├── Serializer.cpp
    └── UndoRedo.cpp
```

---

## 阶段八：完整 2D 游戏能力

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

## 阶段九：Python 脚本集成

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
│   │   ├── Audio/                # 音频子系统 ✅
│   │   ├── Input/                # 输入子系统 ✅
│   │   ├── Math/                 # 数学库 ✅
│   │   ├── SceneGraph/           # 场景图系统（Node / Sprite / Widget...）✅
│   │   ├── Components/           # 组件定义（已废弃）
│   │   ├── AssetManager.h        # 资源缓存 ✅
│   │   ├── Scene.h               # 场景管理 ✅
│   │   └── Timer.h               # 性能计时 ✅
│   ├── Private/                  # 内部实现（.cpp + 内部头文件）
│   │   ├── Platform/
│   │   ├── Render/
│   │   ├── Audio/                ✅
│   │   ├── Input/                ✅
│   │   └── SceneGraph/           ✅
│   ├── PythonBind/               # pybind11 绑定层（阶段九新增）
│   │   ├── PyEngine.cpp
│   │   ├── BindApp.cpp/h
│   │   ├── BindRenderer.cpp/h
│   │   ├── BindInput.cpp/h
│   │   ├── BindScene.cpp/h
│   │   └── BindMath.cpp/h
│   └── CMakeLists.txt
├── scripts/                      # Python 业务脚本（阶段九新增）
│   ├── main.py                   # Python 入口
│   ├── requirements.txt
│   ├── game/
│   ├── scenes/
│   └── components/
├── editor/                       # UI 编辑器 ✅
│   ├── CMakeLists.txt
│   ├── main.cpp                  编辑器入口
│   ├── Public/                   公开 API 头文件
│   │   ├── EditorApp.h
│   │   ├── CanvasView.h
│   │   ├── PropertyPanel.h
│   │   ├── WidgetTreePanel.h
│   │   ├── Toolbar.h
│   │   ├── Gizmo.h
│   │   ├── Serializer.h
│   │   └── UndoRedo.h
│   └── Private/                  内部实现
│       ├── EditorApp.cpp
│       ├── CanvasView.cpp
│       ├── PropertyPanel.cpp
│       ├── WidgetTreePanel.cpp
│       ├── Toolbar.cpp
│       ├── Gizmo.cpp
│       ├── Serializer.cpp
│       └── UndoRedo.cpp
├── sandbox/                      # 测试/示例项目
│   ├── CMakeLists.txt
│   └── main.cpp
└── third_party/                  # 第三方库
    ├── glad/
    ├── glfw/
    ├── stb_image/
    └── miniaudio/
```