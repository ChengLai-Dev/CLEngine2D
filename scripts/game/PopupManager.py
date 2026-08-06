# -*- coding: utf-8 -*-
"""弹窗管理器（PopupManager）。

模型：
- 每个弹窗 = UISystem 全局根下的一棵独立 UI 层（zorder 高于场景 UI 树）
- 独立弹窗（attached_scene=None）：常驻，直到显式 Close
- 从属弹窗（attached_scene=场景名）：仅在该场景为当前场景时可打开；
  切到其他场景时由 main 通知自动销毁（直接摘树，无动画）
- 模态（modal=True）：UISystem 层 modal 机制阻断下层命中（模态层测完无命中也
  不穿透）；modal=False 时点击可穿透到下层
- 销毁延迟：close() 只标记，帧末 update() 统一摘树（避免回调栈内销毁宿主）
- 弹窗打开时下层场景不暂停（on_update 照常）
"""

from CLEngine.SceneGraph import UISystem

# 弹窗 zorder 基准（场景 UI 树为 0，展示浮层 50，弹窗 100+）
POPUP_ZORDER_BASE = 100


class Popup:
    """弹窗基类：构造时挂树，close() 延迟销毁（帧末统一摘树，避免回调栈内销毁宿主）。"""

    def __init__(self, name, cui_path, zorder, modal=True):
        self.name = name
        self.modal = modal
        self.ui_root = UISystem.GetInstance().AddUI(cui_path, zorder, modal)

    def close(self):
        """标记关闭：加入待销毁队列，由 PopupManager 在帧末统一执行 close_now()。"""
        PopupManager.GetInstance()._request_close(self)

    def close_now(self):
        """实际销毁（PopupManager 帧末调用）：摘除 UI 树。"""
        if self.ui_root is not None:
            UISystem.GetInstance().RemoveUI(self.ui_root)
            self.ui_root = None

    def update(self, dt):
        pass

    def on_input(self, events):
        pass


class PopupManager:
    """弹窗注册与生命周期管理（Python 单例）。"""

    _instance = None

    def __init__(self):
        self._defs = {}      # name -> {"cui","zorder","attached_scene","modal","cls"}
        self._opened = []    # 已打开弹窗（打开顺序）
        self._pending_close = []  # 待销毁弹窗（帧末统一执行）
        self._current_scene = None

    @staticmethod
    def GetInstance():
        if PopupManager._instance is None:
            PopupManager._instance = PopupManager()
        return PopupManager._instance

    def register(self, name, cui, zorder=POPUP_ZORDER_BASE,
                 attached_scene=None, modal=True, cls=Popup):
        """注册弹窗定义。attached_scene=None 为独立弹窗（常驻）。"""
        self._defs[name] = {
            "cui": cui, "zorder": zorder,
            "attached_scene": attached_scene,
            "modal": modal, "cls": cls,
        }

    def open(self, name):
        """打开弹窗；从属弹窗且当前场景不匹配时静默返回。"""
        if name in self._opened_names():
            return None
        definition = self._defs.get(name)
        if definition is None:
            return None
        attached = definition["attached_scene"]
        if attached is not None and attached != self._current_scene:
            return None
        popup = definition["cls"](name, definition["cui"],
                                  definition["zorder"], definition["modal"])
        if popup.ui_root is None:
            return None
        self._opened.append(popup)
        return popup

    def close(self, name):
        """关闭指定弹窗（延迟销毁）。"""
        for popup in list(self._opened):
            if popup.name == name:
                self._request_close(popup)
                return True
        return False

    def _request_close(self, popup):
        """加入待销毁队列（幂等）。"""
        if popup not in self._pending_close:
            self._pending_close.append(popup)

    def _flush_pending_close(self):
        """帧末统一销毁：确保不在任何回调栈内销毁弹窗宿主。"""
        while self._pending_close:
            popup = self._pending_close.pop(0)
            popup.close_now()
            if popup in self._opened:
                self._opened.remove(popup)

    def is_open(self, name):
        return name in self._opened_names()

    def on_scene_changed(self, current_scene):
        """场景栈顶变化：销毁所有从属弹窗（独立弹窗保留）。"""
        self._current_scene = current_scene
        for popup in list(self._opened):
            definition = self._defs.get(popup.name)
            attached = definition["attached_scene"] if definition else None
            if attached is not None and attached != current_scene:
                self._request_close(popup)

    def update(self, dt):
        for popup in self._opened:
            popup.update(dt)
        self._flush_pending_close()

    def on_input(self, events):
        """输入分发：最后打开的弹窗先收。"""
        for popup in reversed(self._opened):
            popup.on_input(events)

    def _opened_names(self):
        return {popup.name for popup in self._opened}
