# -*- coding: utf-8 -*-
"""失败画面控制器（GameOver.cui，最小版）。

- "书页合上" 标题 + 提示
- 重试按钮：恢复战前队伍快照 → 弹回 DialogScene → 原参数重 push BattleScene
"""

from CLEngine.SceneGraph import Scene, SceneManager, UISystem

from components.Tweener import Tweener
from game.GameState import GameState


class GameOverScene:
    """失败画面：重试 = 恢复快照 + 重新进入当前战斗。"""

    def __init__(self):
        self.name = "gameover"
        self.scene = Scene()
        self.scene.LoadUI("assets/ui/GameOver.cui")
        SceneManager.GetInstance().PushScene(self.scene)
        self.ui_root = UISystem.GetInstance().GetUIRoot()
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

    # ---------- 重试 ----------

    def _on_retry(self):
        formation_id = self.game_state.formation_id
        if formation_id is None:
            self.main_ref.switch_to("title", None)
            return
        # 恢复战前队伍状态（HP/SP/等级/经验）
        self.game_state.restore_party_snapshot()
        # 弹回 DialogScene（保持战斗前的剧情等待状态），再重 push 当前战斗
        self.main_ref.pop_until("dialog", None)
        self.main_ref.push("battle", {"formation": formation_id})
