# -*- coding: utf-8 -*-
"""战斗结算场景控制器（BattleResult.cui）。

- 胜利标题
- 战利品（经验）打字机输出
- 升级明细（LevelUpLabel 打字机输出，默认隐藏）
- 继续按钮 → 弹回 DialogScene（经 GameState.battle_return_node 恢复剧情）
"""

from CLEngine.SceneGraph import Scene, SceneManager, UISystem

from components.Tweener import Tweener
from components.Typewriter import Typewriter
from game.GameState import GameState

DIALOG_SPEED = 30.0


class ResultScene:
    """结算画面：读 GameState.battle_result 展示。"""

    def __init__(self):
        self.name = "result"
        self.scene = Scene()
        self.scene.LoadUI("assets/ui/BattleResult.cui")
        SceneManager.GetInstance().PushScene(self.scene)
        self.ui_root = UISystem.GetInstance().GetUIRoot()
        self.tweener = Tweener()
        self.game_state = GameState()

        root = self.ui_root
        self.title_label = root.FindChild("TitleLabel")
        self.loot_label = root.FindChild("LootLabel")
        self.level_up_label = root.FindChild("LevelUpLabel")
        self.btn_continue = root.FindChild("BtnContinue")

        self.loot_tw = Typewriter(self.loot_label)
        self.level_tw = Typewriter(self.level_up_label)
        self.state = "idle"

        if self.loot_label is not None:
            from CLEngine.SceneGraph import TextAlign
            self.loot_label.SetHAlign(TextAlign.LEFT)
            self.loot_label.SetLineSpacing(1.5)
        if self.level_up_label is not None:
            from CLEngine.SceneGraph import TextAlign
            self.level_up_label.SetHAlign(TextAlign.LEFT)
            self.level_up_label.SetLineSpacing(1.5)

        if self.btn_continue is not None:
            self.btn_continue.OnClicked(lambda b: self._on_continue())

    # ---------- 场景控制器协议 ----------

    def on_enter(self, params):
        result = self.game_state.battle_result or {}
        exp = result.get("exp", 0)
        self._result = result
        loot_lines = ["获得经验 {} 点".format(exp)]
        self._write_loot(loot_lines)
        level_ups = result.get("level_ups") or []
        if level_ups:
            lines = []
            for lu in level_ups:
                gains = "　".join(
                    "{} +{}".format(name, value)
                    for name, value in lu.get("gains", {}).items())
                lines.append("{} 升至 {} 级！".format(lu.get("name", ""), lu.get("new_level", "")))
                lines.append(gains)
            self._write_level_up(lines)

    def on_update(self, dt):
        self.tweener.update(dt)
        self.loot_tw.update(dt)
        self.level_tw.update(dt)

    def on_input(self, events):
        for kind, data in events:
            if kind == "confirm":
                self._on_continue()

    def on_exit(self):
        self.tweener.clear()
        self._result = None

    # ---------- 展示 ----------

    def _write_loot(self, lines):
        if self.loot_label is None:
            return
        self.state = "typing"
        self.loot_tw.start("\n".join(lines), DIALOG_SPEED,
                           on_done=self._on_loot_done)

    def _on_loot_done(self):
        if self.level_up_label is not None and self.level_up_label.IsVisible():
            self.level_tw.start(self.level_up_label.GetText(), DIALOG_SPEED)
        self.state = "waiting"

    def _write_level_up(self, lines):
        if self.level_up_label is None:
            return
        self.level_up_label.SetVisible(True)
        self.level_up_label.SetText("\n".join(lines))

    # ---------- 继续 ----------

    def _on_continue(self):
        if self.state == "typing":
            self.loot_tw.skip()
            return
        if self.state != "waiting" and self.state != "idle":
            return
        self.main_ref.pop_until("dialog", {"resume_node": self.game_state.battle_return_node})
