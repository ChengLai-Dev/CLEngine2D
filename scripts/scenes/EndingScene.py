# -*- coding: utf-8 -*-
"""结局场景控制器（Ending.cui）。

演出流程（状态机：title / extra / rolling / done）：
1. 读 GameState.ending_id（watch/together，由 DialogScene 的 ending 事件写入）
   → ending.json endings[id]：EndingTitle 显示结局名（淡入）
2. 好感附加演出：extra_require 满足时（watch = flags.json watch_extra，
   together = ending.json extra_require），播放差分段送别台词：
   动态立绘（CreateSprite 淡入停留淡出）+ 台词打字机（D3），全部走 Tweener（D4）
3. Staff 滚屏：StaffRoll 从屏幕底部逐帧上移（D4 演示），结束后 BtnBackToTitle 出现
4. 返回标题：switch_to("title")（重开状态清零由 TitleScene 新游戏 reset_new_game 覆盖）
"""

from CLEngine.Math import Vec2, Vec3
from CLEngine.SceneGraph import (
    Scene, SceneManager, UISystem, LoadTexture,
    CreateSprite, CreateLabel, TextAlign,
)

from components.Tweener import Tweener
from components.Typewriter import Typewriter
from game import DataLoader
from game.GameState import GameState

STAFF_ROLL_DURATION = 16.0     # Staff 滚屏时长（秒）
EXTRA_HOLD_TIME = 1.2          # 附加演出每句停留（秒）


class EndingScene:
    """结局演出：结局名 + 附加演出（可选）+ Staff 滚屏 + 返回标题。"""

    def __init__(self):
        self.name = "ending"
        self.scene = Scene()
        self.scene.LoadUI("assets/ui/Ending.cui")
        SceneManager.GetInstance().PushScene(self.scene)
        self.ui_root = UISystem.GetInstance().GetUIRoot()
        self.tweener = Tweener()
        self.game_state = GameState()

        root = self.ui_root
        self.ending_title = root.FindChild("EndingTitle")
        self.staff_roll = root.FindChild("StaffRoll")
        self.btn_back_to_title = root.FindChild("BtnBackToTitle")

        # 附加演出动态控件（E1：立绘 + 台词，挂 UI 根）
        self._extra_sprite = None
        self._extra_label = None
        self._extra_tw = Typewriter(None)
        self._extra_lines = []
        self._extra_index = 0
        self._staff = []
        self.state = "idle"

        if self.btn_back_to_title is not None:
            self.btn_back_to_title.OnClicked(lambda b: self._on_back_to_title())

    # ---------- 场景控制器协议 ----------

    def on_enter(self, params):
        self._play()

    def on_update(self, dt):
        self.tweener.update(dt)
        self._extra_tw.update(dt)

    def on_input(self, events):
        for kind, data in events:
            if kind == "confirm":
                if self.state == "done":
                    self._on_back_to_title()
                elif self.state == "typing_extra":
                    self._extra_tw.skip()

    def on_exit(self):
        self.tweener.clear()
        self._extra_sprite = None
        self._extra_label = None
        self._extra_lines = []
        self._staff = []
        self.state = "idle"

    # ---------- 结局演出 ----------

    def _play(self):
        ending_id = self.game_state.ending_id or "watch"
        endings = DataLoader.get_endings().get("endings", {})
        ending = endings.get(ending_id)
        if ending is None:
            self._start_staff([])
            return
        self._staff = ending.get("staff") or []
        if self.ending_title is not None:
            self.ending_title.SetText(ending.get("title", ""))
            self.ending_title.SetOpacity(0.0)
            self.tweener.to(self.ending_title, "opacity", 1.0, dur=0.6,
                            ease="quad_out")
        # 附加演出判定：extra_require 满足且有台词时先播送别，否则直接滚屏
        req = ending.get("extra_require")
        extra_lines = ending.get("extra_lines") or []
        with_extra = bool(req) and self.game_state.eval_flag_require(req) \
            and bool(extra_lines)
        if with_extra:
            self._extra_lines = list(extra_lines)
            self._extra_index = 0
        if self.ending_title is not None:
            self.tweener.to(self.ending_title, "opacity", 0.0, dur=0.4,
                            ease="quad_in", delay=1.0,
                            on_finish=lambda: self._after_title(with_extra))
        else:
            self._after_title(with_extra)

    def _after_title(self, with_extra):
        if with_extra:
            self._show_extra_line()
        else:
            self._start_staff()

    # ---------- 好感附加演出（送别台词 + 立绘淡出） ----------

    def _show_extra_line(self):
        if self._extra_index >= len(self._extra_lines):
            self._start_staff()
            return
        line = self._extra_lines[self._extra_index]
        self._extra_index += 1
        # 立绘：动态 Sprite（复用，不重复创建）
        if self._extra_sprite is None:
            self._extra_sprite = CreateSprite(self.ui_root, "EndingExtraPortrait")
            if self._extra_sprite is not None:
                self._extra_sprite.SetContentSize(Vec2(560, 560))
                self._extra_sprite.SetPosition(Vec3(0, 70, 0))
                self._extra_sprite.SetZOrder(20)
                self._extra_sprite.SetVisible(False)
        if self._extra_sprite is not None:
            portraits = DataLoader.get_characters().get("portraits", {})
            entry = portraits.get(line.get("portrait"))
            if entry is not None:
                self._extra_sprite.SetTexture(LoadTexture(entry.get("texture", "")))
            self._extra_sprite.SetVisible(True)
            self._extra_sprite.SetOpacity(0.0)
            self.tweener.to(self._extra_sprite, "opacity", 1.0, dur=0.5,
                            ease="quad_out")
        # 台词：动态 Label（打字机 D3）
        if self._extra_label is None:
            self._extra_label = CreateLabel(self.ui_root, "EndingExtraText")
            if self._extra_label is not None:
                self._extra_label.SetFontSize(26)
                self._extra_label.SetContentSize(Vec2(760, 120))
                self._extra_label.SetPosition(Vec3(0, -300, 0))
                self._extra_label.SetZOrder(21)
                self._extra_label.SetHAlign(TextAlign.CENTER)
                self._extra_label.SetVisible(False)
        if self._extra_label is not None:
            self._extra_label.SetText("")
            self._extra_label.SetVisible(True)
            self._extra_label.SetOpacity(1.0)
            self._extra_tw = Typewriter(self._extra_label)
            self.state = "typing_extra"
            self._extra_tw.start(line.get("text", ""),
                                 self.game_state.get_text_speed(),
                                 on_done=self._on_extra_typed)

    def _on_extra_typed(self):
        self.state = "extra_hold"
        self.tweener.to(self._extra_sprite, "opacity", 0.0, dur=0.5,
                        ease="quad_in", delay=EXTRA_HOLD_TIME,
                        on_finish=self._on_extra_faded)

    def _on_extra_faded(self):
        if self._extra_sprite is not None:
            self._extra_sprite.SetVisible(False)
        if self._extra_label is not None:
            self._extra_label.SetVisible(False)
        self._show_extra_line()

    # ---------- Staff 滚屏 ----------

    def _start_staff(self):
        if self._extra_sprite is not None:
            self._extra_sprite.SetVisible(False)
        if self._extra_label is not None:
            self._extra_label.SetVisible(False)
        if self.staff_roll is None:
            self._on_staff_done()
            return
        self.staff_roll.SetText("\n".join(self._staff))
        self.staff_roll.SetPosition(Vec3(0, -430, 0))
        self.staff_roll.SetOpacity(1.0)
        self.state = "rolling"
        self.tweener.to(self.staff_roll, "position", Vec3(0, 430, 0),
                        dur=STAFF_ROLL_DURATION, ease="linear",
                        on_finish=self._on_staff_done)

    def _on_staff_done(self):
        self.state = "done"
        if self.btn_back_to_title is not None:
            self.btn_back_to_title.SetVisible(True)
            self.btn_back_to_title.SetOpacity(0.0)
            self.tweener.to(self.btn_back_to_title, "opacity", 1.0, dur=0.5,
                            ease="quad_out")

    # ---------- 返回标题 ----------

    def _on_back_to_title(self):
        self.main_ref.switch_to("title", None)
