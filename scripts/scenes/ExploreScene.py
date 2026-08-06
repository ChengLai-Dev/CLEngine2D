# -*- coding: utf-8 -*-
"""探索场景控制器（ExploreScene.cui，选择式探索）。

数据驱动（data/scenes.json）：
- 背景纹理 / 场景标题 / 热区（Hotspot0~2：文本/动作/目标）
- 明雷敌人（EnemySprite + EnemyName：可见可点击，触发编成战斗；绕行 = 不点）
- 返回节点（BtnBack → 回 DialogScene 从进入探索的下一节点继续，不循环触发 enter_scene）

热区动作两种：
- dialogue：pop 回 DialogScene 指定节点（target）
- battle：事件触发战（记录 battle_source="explore"，push BattleScene 对应编成）

明雷触发同 battle 协议；visited_{formation}=1 后隐藏明雷与事件触发热区。
"""

from CLEngine.Math import Vec2, Vec3
from CLEngine.SceneGraph import (
    Scene, SceneManager, UISystem, LoadTexture,
    CreateButton,
)

from components.Tweener import Tweener
from game import DataLoader
from game.GameState import GameState

HOTSPOT_COUNT = 3
ENEMY_TOUCH_SIZE = 160.0


class ExploreScene:
    """探索场景控制器。"""

    def __init__(self):
        self.name = "explore"
        self.scene = Scene()
        SceneManager.GetInstance().PushScene(self.scene)
        self.ui_root = UISystem.GetInstance().AddUI("assets/ui/ExploreScene.cui", 0)
        self.tweener = Tweener()
        self.game_state = GameState()

        root = self.ui_root
        self.bg_scene = root.FindChild("BgScene")
        self.scene_title = root.FindChild("SceneTitle")
        self.btn_back = root.FindChild("BtnBack")
        self.hotspots = [root.FindChild("Hotspot{}".format(i)) for i in range(HOTSPOT_COUNT)]
        self.enemy_sprite = root.FindChild("EnemySprite")
        self.enemy_name = root.FindChild("EnemyName")
        self._scene_id = None
        self._config = None
        self._tex_cache = {}
        # 明雷点击区：Sprite 无触摸绑定（引擎约束），动态叠一层透明按钮
        # （位置跟随 EnemySprite，运行时显隐，不修改 .cui）
        self._enemy_touch_btn = None
        if self.enemy_sprite is not None:
            btn = CreateButton(root, "EnemyTouch")
            if btn is not None:
                pos = self.enemy_sprite.GetPosition()
                btn.SetContentSize(Vec2(ENEMY_TOUCH_SIZE, ENEMY_TOUCH_SIZE))
                btn.SetPosition(Vec3(pos.x, pos.y, 0))
                btn.SetZOrder(4)
                btn.SetVisible(False)
                btn.OnClicked(lambda b: self._on_enemy_touched())
                self._enemy_touch_btn = btn

        if self.btn_back is not None:
            self.btn_back.OnClicked(lambda b: self._on_back())
        for i, spot in enumerate(self.hotspots):
            if spot is not None:
                spot.OnClicked(lambda b, idx=i: self._on_hotspot(idx))

    # ---------- 场景控制器协议 ----------

    def on_enter(self, params):
        params = params or {}
        scene_id = params.get("scene")
        if scene_id is None:
            scene_id = self.game_state.dialogue_node or "forest"
        self._scene_id = scene_id
        self._config = DataLoader.get_scenes().get(scene_id)
        if self._config is None:
            self._on_back()
            return
        self._setup_scene()

    def on_update(self, dt):
        self.tweener.update(dt)

    def on_input(self, events):
        for kind, data in events:
            if kind == "confirm":
                self._on_back()

    def on_exit(self):
        self.tweener.clear()
        if self.ui_root is not None:
            UISystem.GetInstance().RemoveUI(self.ui_root)
            self.ui_root = None
        self._scene_id = None
        self._config = None
        self._tex_cache = {}
        self._enemy_touch_btn = None

    # ---------- 场景布置（数据驱动） ----------

    def _setup_scene(self):
        config = self._config
        if self.bg_scene is not None:
            tex = self._load_tex(config.get("bg"))
            if tex is not None:
                self.bg_scene.SetTexture(tex)
        if self.scene_title is not None:
            self.scene_title.SetText(config.get("title", ""))

        hotspots = config.get("hotspots", [])
        for i, spot in enumerate(self.hotspots):
            if spot is None:
                continue
            if i < len(hotspots):
                spot.SetVisible(True)
                spot.SetText(hotspots[i].get("text", ""))
            else:
                spot.SetVisible(False)

        # 明雷敌人：已打完（visited）则隐藏；否则可见可点击
        enemy = config.get("enemy")
        visited = False
        if enemy is not None:
            formation_id = enemy.get("formation")
            visited = self.game_state.get_flag("visited_" + formation_id) > 0
        if enemy is not None and not visited:
            if self.enemy_sprite is not None:
                tex = self._load_tex(enemy.get("texture"))
                if tex is not None:
                    self.enemy_sprite.SetTexture(tex)
                self.enemy_sprite.SetVisible(True)
            if self.enemy_name is not None:
                self.enemy_name.SetText(enemy.get("name", ""))
                self.enemy_name.SetVisible(True)
            if self._enemy_touch_btn is not None:
                self._enemy_touch_btn.SetVisible(True)
        else:
            if self.enemy_sprite is not None:
                self.enemy_sprite.SetVisible(False)
            if self.enemy_name is not None:
                self.enemy_name.SetVisible(False)
            if self._enemy_touch_btn is not None:
                self._enemy_touch_btn.SetVisible(False)

        # 事件触发热区（action=battle）同样受 visited 控制
        for i, h in enumerate(hotspots):
            if i >= len(self.hotspots) or self.hotspots[i] is None:
                continue
            if h.get("action") == "battle" and visited:
                self.hotspots[i].SetVisible(False)

    # ---------- 交互 ----------

    def _on_hotspot(self, index):
        config = self._config
        if config is None:
            return
        hotspots = config.get("hotspots", [])
        if not (0 <= index < len(hotspots)):
            return
        action = hotspots[index].get("action")
        if action == "battle":
            self._trigger_battle(hotspots[index].get("formation"))
        else:
            target = hotspots[index].get("target") or config.get("return_node")
            self.main_ref.pop({"resume_node": target})

    def _on_enemy_touched(self):
        config = self._config
        if config is None:
            return
        enemy = config.get("enemy")
        if enemy is None:
            return
        formation_id = enemy.get("formation")
        if self.game_state.get_flag("visited_" + formation_id) > 0:
            return
        self._trigger_battle(formation_id)

    def _on_back(self):
        config = self._config
        target = None
        if config is not None:
            target = config.get("return_node")
        if not target:
            target = self.game_state.explore_return_node
        params = {"resume_node": target} if target else None
        self.main_ref.pop(params)

    def _trigger_battle(self, formation_id):
        """明雷/事件触发战：记录 battle_source="explore"，push BattleScene。"""
        formations = DataLoader.get_formations()
        formation = formations.get(formation_id)
        if formation is None:
            self._on_back()
            return
        self.game_state.battle_source = "explore"
        self.game_state.set_battle_return(formation.get("victory"))
        self.main_ref.push("battle", {"formation": formation_id})

    # ---------- 工具 ----------

    def _load_tex(self, path):
        if not path:
            return None
        if path in self._tex_cache:
            return self._tex_cache[path]
        tex = LoadTexture(path)
        self._tex_cache[path] = tex
        return tex
