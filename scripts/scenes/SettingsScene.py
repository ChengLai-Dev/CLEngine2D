# -*- coding: utf-8 -*-
"""设置弹窗（Settings.cui，PopupManager 管理，不再 push 场景）。

- 文字速度三档真实生效（D-07）：慢/中/快 = 20/30/45 字/秒，写入 GameState.settings，
  倍率作用于全部打字机（对话/战斗插话/结算明细统一走 GameState.get_text_speed）
- 音量条灰置态：E3 音频未实施，BgmRow/SfxRow 降低透明度 + 文本标注「（未实装）」
  （Image 无 SetInteractable，用透明度+文案演示置灰；.cui 零改动）
- 键盘导航（D7）：confirm → 关闭弹窗
- 接入：TitleScene BtnSettings → PopupManager.open("settings")；BtnBack → close
- 模态：UISystem 层 modal 机制阻断下层点击；面板不透明由动态纯色垫底
  （SettingsBg，panel_9slice.png 中心透明，.cui 零改动）
"""

from CLEngine.Math import Vec2, Vec4
from CLEngine.SceneGraph import CreateImage

from components.Tweener import Tweener
from game.GameState import GameState, SPEED_PRESETS
from game.PopupManager import Popup

# 档位按钮（控件契约）：慢/中/快 = 20/30/45
SPEED_BUTTONS = {"slow": "BtnSpeedSlow", "normal": "BtnSpeedNormal", "fast": "BtnSpeedFast"}
GRAY_ALPHA = 0.15          # 音量条灰置透明度
BAR_ALPHA = 0.4            # .cui 原始透明度
PANEL_SIZE = (640.0, 420.0)          # SettingsPanel 尺寸
PANEL_BG_COLOR = (0.08, 0.08, 0.12)  # 面板垫底色（不透明）


class SettingsPopup(Popup):
    """设置弹窗：文字速度三档 + 音量条灰置演示。"""

    def __init__(self, name, cui_path, zorder, modal):
        super().__init__(name, cui_path, zorder, modal)
        self.tweener = Tweener()
        self.game_state = GameState()

        root = self.ui_root
        # 面板垫底：panel_9slice.png 中心透明，先画不透明纯色底（zOrder 低于 .cui 树）
        bg = CreateImage(root, "SettingsBg")
        if bg is not None:
            bg.SetContentSize(Vec2(PANEL_SIZE[0], PANEL_SIZE[1]))
            bg.SetZOrder(-1)
            bg.SetColor(Vec4(PANEL_BG_COLOR[0], PANEL_BG_COLOR[1], PANEL_BG_COLOR[2], 1.0))
        self.bgm_row = root.FindChild("BgmRow")
        self.bgm_label = root.FindChild("BgmLabel")
        self.bgm_bar_bg = root.FindChild("BgmBarBg")
        self.bgm_bar_fill = root.FindChild("BgmBarFill")
        self.sfx_row = root.FindChild("SfxRow")
        self.sfx_label = root.FindChild("SfxLabel")
        self.sfx_bar_bg = root.FindChild("SfxBarBg")
        self.sfx_bar_fill = root.FindChild("SfxBarFill")
        self.speed_buttons = {}
        for preset, control_name in SPEED_BUTTONS.items():
            btn = root.FindChild(control_name)
            if btn is not None:
                btn.OnClicked(lambda b, p=preset: self._on_speed_chosen(p))
            self.speed_buttons[preset] = btn
        self.btn_back = root.FindChild("BtnBack")
        if self.btn_back is not None:
            self.btn_back.OnClicked(lambda b: self.close())

        self._apply_gray_volume()
        self._refresh_speed_buttons()

    def close_now(self):
        """实际销毁（PopupManager 帧末调用）：摘树 + 清理引用。"""
        super().close_now()
        self.tweener.clear()
        self.speed_buttons = {}
        self.btn_back = None

    def update(self, dt):
        self.tweener.update(dt)

    def on_input(self, events):
        for kind, data in events:
            if kind == "confirm":
                self.close()

    # ---------- 音量条灰置态（E3 未实施） ----------

    def _apply_gray_volume(self):
        for node in (self.bgm_bar_bg, self.bgm_bar_fill,
                     self.sfx_bar_bg, self.sfx_bar_fill):
            if node is not None:
                node.SetOpacity(GRAY_ALPHA)
        if self.bgm_label is not None:
            self.bgm_label.SetText("BGM 音量　80%（未实装）")
        if self.sfx_label is not None:
            self.sfx_label.SetText("音效音量　70%（未实装）")

    # ---------- 文字速度三档 ----------

    def _on_speed_chosen(self, preset):
        self.game_state.set_text_speed(preset)
        self._refresh_speed_buttons()

    def _refresh_speed_buttons(self):
        """当前档位高亮（透明度 1.0），其余档位半透明（0.5）。"""
        current = self.game_state.settings.get("text_speed", 30)
        for preset, btn in self.speed_buttons.items():
            if btn is None:
                continue
            value = SPEED_PRESETS.get(preset, 30)
            btn.SetOpacity(1.0 if value == current else 0.5)
