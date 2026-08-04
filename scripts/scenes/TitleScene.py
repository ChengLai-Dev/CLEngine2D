# -*- coding: utf-8 -*-
"""标题场景控制器（Title.cui）。

- 书页粒子漂浮动画（Tweener 位移 + 透明度）
- 新游戏 → 重置 GameState → switch_to DialogScene
- 读取存档按钮保持灰置（.cui 已 enabled=false，不动）
- 退出 → 直接结束进程（引擎无退出 API，演示用 os._exit）
"""

import os

from CLEngine.Math import Vec3
from CLEngine.Renderer import SetClearColor
from CLEngine.SceneGraph import Scene, SceneManager, UISystem

from components.Tweener import Tweener
from game.GameState import GameState


class TitleScene:
    """标题画面：粒子漂浮 + 菜单按钮。"""

    def __init__(self):
        self.name = "title"
        self.scene = Scene()
        self.scene.LoadUI("assets/ui/Title.cui")
        SceneManager.GetInstance().PushScene(self.scene)
        self.ui_root = UISystem.GetInstance().GetUIRoot()
        self.tweener = Tweener()
        self.particles = []

        root = self.ui_root
        self.btn_new_game = root.FindChild("BtnNewGame")
        self.btn_settings = root.FindChild("BtnSettings")
        self.btn_quit = root.FindChild("BtnQuit")
        self.particle_nodes = [
            root.FindChild("PageParticle0"),
            root.FindChild("PageParticle1"),
            root.FindChild("PageParticle2"),
        ]

        if self.btn_new_game is not None:
            self.btn_new_game.OnClicked(lambda btn: self._on_new_game())
        if self.btn_settings is not None:
            self.btn_settings.OnClicked(lambda btn: self._on_settings())
        if self.btn_quit is not None:
            self.btn_quit.OnClicked(lambda btn: os._exit(0))

        self._start_particle_anim()

    def _start_particle_anim(self):
        """三片书页粒子：位置往返漂浮 + 透明度呼吸。"""
        drift = [
            (Vec3(500.0, 250.0, 0.0), 3.0),
            (Vec3(460.0, 130.0, 0.0), 2.5),
            (Vec3(-530.0, 200.0, 0.0), 3.2),
        ]
        for i, node in enumerate(self.particle_nodes):
            if node is None:
                continue
            end_pos, dur = drift[i]
            self.tweener.to(node, "position", end_pos, dur=dur, ease="sine_out",
                            pingpong=True)
            self.tweener.to(node, "opacity", 0.35, dur=dur * 0.5,
                            ease="sine_out", pingpong=True)

    def _on_new_game(self):
        GameState().reset_new_game()
        self.main_ref.switch_to("dialog", {"file": "prologue", "node": "pro_001"})

    def _on_settings(self):
        self.main_ref.push("settings", None)

    # ---------- 场景控制器协议 ----------

    def on_enter(self, params):
        pass

    def on_update(self, dt):
        self.tweener.update(dt)

    def on_input(self, events):
        pass

    def on_exit(self):
        self.tweener.clear()
        self.particle_nodes = []
        self.btn_new_game = None
        self.btn_settings = None
        self.btn_quit = None
