# -*- coding: utf-8 -*-
"""《星环遗章》阶段 3 主入口。

职责：
- 初始化 GameState、注册场景控制器、F5 热重载、每帧调度
- 控制器栈与 SceneManager 栈一一对应：push / pop / pop_until / switch_to
- 关键：PopScene 后 UISystem 的 UI 树根需手动指回新栈顶场景
  （UISystem 是全局单树，引擎不会自动恢复，否则旧场景销毁后悬垂）
- 全局输入（InputSystem）：Confirm（空格/回车）、Digit1~4（数字键选选项）、F5 热重载
  边沿检测后以事件列表分发给栈顶控制器 on_input
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

# 场景注册表（name -> 控制器类）
SCENE_REGISTRY = {
    "title": TitleScene,
    "dialog": DialogScene,
    "battle": BattleScene,
    "result": ResultScene,
    "gameover": GameOverScene,
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

def push(name, params=None):
    """压入新场景：实例化控制器（构造内 LoadUI + PushScene）→ on_enter。"""
    cls = SCENE_REGISTRY[name]
    ctrl = cls()
    ctrl.main_ref = sys.modules[__name__]
    _stack.append(ctrl)
    ctrl.on_enter(params)
    return ctrl


def pop(params=None):
    """弹出栈顶场景；PopScene 后把 UISystem 根指回新栈顶（悬垂防护）。"""
    if not _stack:
        return None
    ctrl = _stack.pop()
    ctrl.on_exit()
    SceneManager.GetInstance().PopScene()
    if _stack:
        UISystem.GetInstance().SetUIRoot(_stack[-1].ui_root)
        if params is not None:
            _stack[-1].on_enter(params)
    return ctrl


def pop_until(name, params=None):
    """连续弹出直到指定控制器成为栈顶（弹栈顺序后，栈顶 on_enter(params)）。"""
    while _stack and _stack[-1].name != name:
        ctrl = _stack.pop()
        ctrl.on_exit()
        SceneManager.GetInstance().PopScene()
    if not _stack:
        return None
    UISystem.GetInstance().SetUIRoot(_stack[-1].ui_root)
    if params is not None:
        _stack[-1].on_enter(params)
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
    push("title", None)


def on_update(dt):
    UISystem.GetInstance().ProcessEvents()
    events = _collect_input_events()
    if _stack:
        _stack[-1].on_update(dt)
        if events:
            _stack[-1].on_input(events)


def on_render():
    pass


def on_shutdown():
    print("[main] shutdown")
