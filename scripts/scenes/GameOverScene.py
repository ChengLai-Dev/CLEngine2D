# -*- coding: utf-8 -*-
"""失败画面控制器（GameOver.cui）。

- "书页合上" 标题 + 提示
- 重试按钮：恢复战前快照（队伍 + 道具）→ 弹回战前场景 → 原参数重 push BattleScene
- 战前场景按 battle_source 分叉：dialog → pop_until("dialog")；explore → pop_until("explore")
  （明雷战失败重试后 ExploreScene 状态不丢）
"""

from CLEngine.SceneGraph import Scene, SceneManager, UISystem

from components.Tweener import Tweener
from game.GameState import GameState


class GameOverScene:
    """失败画面：重试 = 恢复快照 + 重新进入当前战斗。"""

    def __init__(self):
        self.name = "gameover"
        self.scene = Scene()
        SceneManager.GetInstance().PushScene(self.scene)
        self.ui_root = UISystem.GetInstance().AddUI("assets/ui/GameOver.cui", 0)
        self.tweener = Tweener()
        self.game_state = GameState()

        root = self.ui_root
        self.btn_retry = root.FindChild("BtnRetry")
        if self.btn_retry is not None:
            self.btn_retry.OnClicked(lambda b: self._on_retry())

    # ---------- 场景控制器协议 ----------

    def on_enter(self, params):
        pass

    def on_update(self, dt):
        self.tweener.update(dt)

    def on_input(self, events):
        for kind, data in events:
            if kind == "confirm":
                self._on_retry()

    def on_exit(self):
        self.tweener.clear()
        if self.ui_root is not None:
            UISystem.GetInstance().RemoveUI(self.ui_root)
            self.ui_root = None

    # ---------- 重试 ----------

    def _on_retry(self):
        formation_id = self.game_state.formation_id
        if formation_id is None:
            self.main_ref.switch_to("title", None)
            return
        # 恢复战前队伍状态（HP/SP/等级/经验 + 道具数量）
        self.game_state.restore_party_snapshot()
        # 弹回战前场景（明雷战回 ExploreScene，剧情战回 DialogScene），再重 push 当前战斗
        target = "explore" if self.game_state.battle_source == "explore" else "dialog"
        self.main_ref.pop_until(target, None)
        self.main_ref.push("battle", {"formation": formation_id})
