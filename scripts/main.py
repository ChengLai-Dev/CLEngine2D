# -*- coding: utf-8 -*-
"""《星环遗章》阶段 3 主入口。

职责：
- 初始化 GameState、注册场景控制器、F5 热重载、每帧调度
- 控制器栈与 SceneManager 栈一一对应：push / pop / pop_until / switch_to
- 场景 UI 树挂摘：控制器构造时 UISystem.AddUI 挂树、on_exit 时 RemoveUI 摘树；
  push 时隐藏旧栈顶场景 UI 树，pop/pop_until 恢复显示（UISystem 多树叠加，非栈顶不显示）
- 弹窗（PopupManager）：覆盖页/浮层走 UISystem 层，不 push 场景；场景切换时
  自动销毁从属弹窗
- 全局输入（InputSystem）：Confirm（空格/回车）、Digit1~4（数字键选选项）、F5 热重载
  边沿检测后以事件列表分发给栈顶控制器与弹窗管理器
"""

import sys

import debugpy
debugpy.connect(("localhost", 5678))
debugpy.wait_for_client()

from CLEngine.Math import Vec2
from CLEngine.Input import InputAction, InputMappingContext, InputSystem, KeyCode
from CLEngine.Renderer import SetClearColor
from CLEngine.SceneGraph import SceneManager, UISystem
import CLEngine

from scenes.TitleScene import TitleScene
from scenes.DialogScene import DialogScene
from scenes.BattleScene import BattleScene
from scenes.ResultScene import ResultScene
from scenes.GameOverScene import GameOverScene
from scenes.ExploreScene import ExploreScene
from scenes.EndingScene import EndingScene
from scenes.SettingsScene import SettingsPopup
from game.PopupManager import PopupManager

# 场景注册表（name -> 控制器类）；覆盖页/浮层改走 PopupManager，不入注册表
SCENE_REGISTRY = {
    "title": TitleScene,
    "dialog": DialogScene,
    "battle": BattleScene,
    "result": ResultScene,
    "gameover": GameOverScene,
    "explore": ExploreScene,
    "ending": EndingScene,
}

_stack = []          # 控制器栈（与 SceneManager 栈一一对应）

# 全局输入 Action（InputSystem；InputMode 保持默认 GameOnly）
_confirm_action = InputAction()
_digit_actions = [InputAction() for _ in range(4)]
_prev_confirm = False
_prev_digits = [False] * 4

_reload_ctx = None


def _setup_input():
    """注册全局输入上下文：Confirm / 数字键 / F5 热重载。"""
    global _reload_ctx
    ctx = InputMappingContext()
    ctx.MapKey(_confirm_action, KeyCode.Space)
    ctx.MapKey(_confirm_action, KeyCode.Enter)
    digit_keys = [KeyCode.Number1, KeyCode.Number2, KeyCode.Number3, KeyCode.Number4]
    for i, action in enumerate(_digit_actions):
        ctx.MapKey(action, digit_keys[i])
    InputSystem.GetInstance().AddContext(ctx)

    if _reload_ctx is not None:
        InputSystem.GetInstance().RemoveContext(_reload_ctx)
    action = InputAction()
    action.OnTriggered(lambda value: CLEngine.ReloadScripts())
    _reload_ctx = InputMappingContext()
    _reload_ctx.MapKey(action, KeyCode.F5, Vec2(1, 0))
    InputSystem.GetInstance().AddContext(_reload_ctx)


# ---------- 控制器栈 ----------

def _set_scene_ui_visible(ctrl, visible):
    """场景 UI 树显隐（UISystem 多树叠加：非栈顶场景的 UI 树不显示不命中）。"""
    ui_root = getattr(ctrl, "ui_root", None)
    if ui_root is not None:
        ui_root.SetVisible(visible)


def push(name, params=None):
    """压入新场景：实例化控制器（构造内 AddUI 挂树 + PushScene）→ 隐藏旧栈顶 UI → on_enter。"""
    cls = SCENE_REGISTRY[name]
    ctrl = cls()
    ctrl.main_ref = sys.modules[__name__]
    if _stack:
        _set_scene_ui_visible(_stack[-1], False)
    _stack.append(ctrl)
    ctrl.on_enter(params)
    PopupManager.GetInstance().on_scene_changed(ctrl.name)
    return ctrl


def pop(params=None):
    """弹出栈顶场景；on_exit 摘除其 UI 树后，恢复新栈顶场景的 UI 树显示。"""
    if not _stack:
        return None
    ctrl = _stack.pop()
    ctrl.on_exit()
    SceneManager.GetInstance().PopScene()
    if _stack:
        _set_scene_ui_visible(_stack[-1], True)
        if params is not None:
            _stack[-1].on_enter(params)
        PopupManager.GetInstance().on_scene_changed(_stack[-1].name)
    return ctrl


def pop_until(name, params=None):
    """连续弹出直到指定控制器成为栈顶（弹栈顺序后，栈顶 on_enter(params)）。"""
    while _stack and _stack[-1].name != name:
        ctrl = _stack.pop()
        ctrl.on_exit()
        SceneManager.GetInstance().PopScene()
    if not _stack:
        return None
    _set_scene_ui_visible(_stack[-1], True)
    if params is not None:
        _stack[-1].on_enter(params)
    PopupManager.GetInstance().on_scene_changed(_stack[-1].name)
    return _stack[-1]


def switch_to(name, params=None):
    """清栈换场景（标题 → 新游戏等）。"""
    while _stack:
        ctrl = _stack.pop()
        ctrl.on_exit()
        SceneManager.GetInstance().PopScene()
    return push(name, params)


# ---------- 主循环回调 ----------

def _collect_input_events():
    """InputSystem Action 边沿检测（Advance 在引擎 OnUpdate 之后，读到的
    是上一帧状态，延迟 1 帧可忽略）。"""
    global _prev_confirm
    events = []
    now = _confirm_action.IsActive()
    if now and not _prev_confirm:
        events.append(("confirm", None))
    _prev_confirm = now
    for i, action in enumerate(_digit_actions):
        now = action.IsActive()
        if now and not _prev_digits[i]:
            events.append(("digit", i))
        _prev_digits[i] = now
    return events


def on_init():
    SetClearColor(0.1, 0.1, 0.15, 1.0)
    _setup_input()
    PopupManager.GetInstance().register("settings", "assets/ui/Settings.cui",
                                        cls=SettingsPopup)
    push("title", None)


def on_update(dt):
    UISystem.GetInstance().ProcessEvents()
    events = _collect_input_events()
    if _stack:
        _stack[-1].on_update(dt)
        if events:
            _stack[-1].on_input(events)
    PopupManager.GetInstance().update(dt)
    if events:
        PopupManager.GetInstance().on_input(events)


def on_render():
    pass


def on_shutdown():
    print("[main] shutdown")
