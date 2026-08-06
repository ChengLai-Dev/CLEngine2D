# -*- coding: utf-8 -*-
"""剧情对话场景控制器（DialogScene.cui）。

功能：
- 打字机（Typewriter；速度 = 节点 typewriter 优先，缺省 Settings 档位，阶段 5 前默认中速 30）
- 立绘三站位（characters.json poses 显隐面板 + 换图淡入）
- 名牌（无 speaker 隐藏）
- 推进箭头闪烁（Tweener 透明度往返）
- 选项（E1 CreateButton 动态挂 OptionLayout + DoLayout；鼠标/数字键选择；选项显示时推进键无效）
- 推进交互（D-09）：空格/回车（InputSystem Confirm action）/ 鼠标左键点击对话框任意处；
  打字中按推进键跳完整句
- 事件分发：battle_start（保存返回指针 + push BattleScene）/ battle_end（正常显示，战斗胜利后
  经 resume_node 回到此处）/ chapter（AddUI 叠加 ChapterTransition.cui 淡入淡出后继续）
"""

from CLEngine.Math import Vec2, Vec3
from CLEngine.SceneGraph import (
    Scene, SceneManager, UISystem,
    CreateButton, LoadTexture,
    TextAlign,
)

from components.Tweener import Tweener
from components.Typewriter import Typewriter
from game import DataLoader
from game.DialogueEngine import DialogueEngine
from game.GameState import GameState

# 选项按钮样式（对齐预置 Option0~3）
OPTION_W = 480.0
OPTION_H = 64.0
PORTRAIT_FADE = 0.35         # 立绘换图淡入时长（秒）

_POSES = ("left", "center", "right")


class DialogScene:
    """剧情对话控制器。状态机：idle / typing / waiting / options / event。"""

    def __init__(self):
        self.name = "dialog"
        self.scene = Scene()
        SceneManager.GetInstance().PushScene(self.scene)
        self.ui_root = UISystem.GetInstance().AddUI("assets/ui/DialogScene.cui", 0)
        self.tweener = Tweener()
        self.dialog = DialogueEngine(GameState())
        self.game_state = GameState()

        root = self.ui_root
        self.bg_scene = root.FindChild("BgScene")
        self.portrait_panels = {
            "left": root.FindChild("PortraitLeft"),
            "center": root.FindChild("PortraitCenter"),
            "right": root.FindChild("PortraitRight"),
        }
        self.portrait_sprites = {
            "left": root.FindChild("SpritePortraitLeft"),
            "center": root.FindChild("SpritePortraitCenter"),
            "right": root.FindChild("SpritePortraitRight"),
        }
        self.name_plate = root.FindChild("NamePlate")
        self.name_label = root.FindChild("NameLabel")
        self.portrait_mini = root.FindChild("PortraitMini")
        self.dialogue_text = root.FindChild("DialogueText")
        self.arrow_next = root.FindChild("ArrowNext")
        self.option_panel = root.FindChild("OptionPanel")
        self.option_layout = root.FindChild("OptionLayout")
        self.option_slots = [root.FindChild("Option{}".format(i)) for i in range(4)]

        # 文本排版：左对齐 + 行距 1.6（E2，阶段 1 能力）
        if self.dialogue_text is not None:
            self.dialogue_text.SetHAlign(TextAlign.LEFT)
            self.dialogue_text.SetLineSpacing(1.6)

        self.typewriter = Typewriter(self.dialogue_text)
        self.state = "idle"
        self._option_buttons = []          # E1 动态选项按钮（复用，不重复创建）
        self._tex_cache = {}               # 立绘纹理缓存（避免重复 LoadTexture）
        self._chapter_overlay = None       # 章节过场叠加树根（RemoveUI 后悬垂，勿再用）
        self._waiting_event = False        # 当前节点带事件，推进时触发

        # 预置选项按钮隐藏；高度压到 4px 占位（DoLayout 不跳过隐藏节点，
        # 压扁后动态按钮恰好落在面板上部，见 Layout 布局公式）
        if self.option_layout is not None:
            self.option_layout.SetSpacing(0.0)
        for slot in self.option_slots:
            if slot is not None:
                slot.SetVisible(False)
                slot.SetContentSize(Vec2(OPTION_W, 4.0))
        if self.option_panel is not None:
            self.option_panel.SetVisible(False)
        if self.arrow_next is not None:
            self.arrow_next.SetVisible(False)
        if self.portrait_mini is not None:
            self.portrait_mini.SetVisible(False)
        for pose in _POSES:
            if self.portrait_panels.get(pose) is not None:
                self.portrait_panels[pose].SetVisible(False)

        # 鼠标点击对话框任意处推进（命中测试：上层 touchEnabled 控件自动拦截）
        if root is not None:
            root.OnTouchStarted(lambda widget, pos: self._on_click_advance())

    # ---------- 场景控制器协议 ----------

    def on_enter(self, params):
        params = params or {}
        resume_node = params.get("resume_node")
        if resume_node:
            # 战斗胜利/探索返回：从指定节点继续
            self.dialog.start(self.game_state.dialogue_file, resume_node)
            self._show_current()
            return
        file_name = params.get("file")
        node_id = params.get("node")
        if file_name and node_id:
            self.dialog.start(file_name, node_id)
            self._show_current()

    def on_update(self, dt):
        self.tweener.update(dt)
        self.typewriter.update(dt)

    def on_input(self, events):
        for kind, data in events:
            if kind == "confirm":
                self._on_confirm()
            elif kind == "digit" and self.state == "options":
                self._choose_option(data)

    def on_exit(self):
        self.tweener.clear()
        # 章节过渡层兜底摘除（正常流程 _chapter_done 已摘；退出时可能还在淡入淡出）
        if self._chapter_overlay is not None:
            UISystem.GetInstance().RemoveUI(self._chapter_overlay)
        if self.ui_root is not None:
            UISystem.GetInstance().RemoveUI(self.ui_root)
            self.ui_root = None
        self._option_buttons = []
        self._chapter_overlay = None
        self.state = "idle"

    # ---------- 推进交互（D-09） ----------

    def _on_confirm(self):
        if self.state == "options":
            return                       # 选项显示时推进键无效
        if self.typewriter.is_typing():
            self.typewriter.skip()       # 打字中：跳完整句
            return
        if self.state == "waiting":
            self._on_advance()

    def _on_click_advance(self):
        self._on_confirm()

    def _on_advance(self):
        """推进：事件节点触发事件，普通节点进入下一节点。"""
        event, event_param = self.dialog.get_event()
        if event == "battle_start":
            self._trigger_battle(event_param)
            return
        if event == "enter_scene":
            self._trigger_explore(event_param)
            return
        if event == "chapter":
            self._trigger_chapter(event_param)
            return
        if event == "ending":
            # 结局演出：记录结局 ID（EndingScene 读取）→ push EndingScene
            self.game_state.ending_id = event_param
            self.state = "event"
            self.main_ref.push("ending", {})
            return
        self.dialog.advance()
        self._show_current()

    # ---------- 节点渲染 ----------

    def _show_current(self):
        node = self.dialog.get_node()
        if node is None:
            self._end_of_dialogue()
            return
        self._apply_portrait(node)
        self._apply_name(node)
        self._apply_text(node)
        self._hide_options()
        self._waiting_event = node.get("event") is not None

    def _apply_text(self, node):
        if self.dialogue_text is None:
            return
        # 速度：节点 typewriter 显式优先；缺省用 Settings 档位（D-07 统一入口）
        speed = self.game_state.get_text_speed(node)
        self.state = "typing"
        self.typewriter.start(node.get("text", ""), speed, on_done=self._on_typed_done)
        if self.arrow_next is not None:
            self.arrow_next.SetVisible(False)

    def _on_typed_done(self):
        node = self.dialog.get_node()
        if node is not None and node.get("options"):
            self.state = "options"
            self._show_options()
            return
        self.state = "waiting"
        if self.arrow_next is not None:
            self.arrow_next.SetVisible(True)
            self.tweener.to(self.arrow_next, "opacity", 0.2, dur=0.6,
                            ease="sine_out", pingpong=True)

    def _apply_name(self, node):
        speaker = node.get("speaker")
        if self.name_plate is not None:
            self.name_plate.SetVisible(bool(speaker))
        if self.name_label is not None:
            self.name_label.SetText(speaker or "")

    def _apply_portrait(self, node):
        portrait = node.get("portrait")
        pose = node.get("pose")
        for p in _POSES:
            panel = self.portrait_panels.get(p)
            if panel is not None:
                panel.SetVisible(False)
        if not portrait or not pose:
            return
        panel = self.portrait_panels.get(pose)
        sprite = self.portrait_sprites.get(pose)
        if panel is None or sprite is None:
            return
        texture_path = self._portrait_texture(portrait)
        if texture_path is not None:
            sprite.SetTexture(texture_path)
        panel.SetVisible(True)
        sprite.SetOpacity(0.0)
        self.tweener.to(sprite, "opacity", 1.0, dur=PORTRAIT_FADE, ease="quad_out")

    def _portrait_texture(self, portrait_id):
        """立绘纹理（缓存）：characters.json portraits[].texture。"""
        if portrait_id in self._tex_cache:
            return self._tex_cache[portrait_id]
        portraits = DataLoader.get_characters().get("portraits", {})
        entry = portraits.get(portrait_id)
        if entry is None:
            self._tex_cache[portrait_id] = None
            return None
        tex = LoadTexture(entry.get("texture", ""))
        self._tex_cache[portrait_id] = tex
        return tex

    def _end_of_dialogue(self):
        """当前对话文件到头：回到标题。"""
        self.state = "idle"
        self.main_ref.switch_to("title", None)

    # ---------- 选项（E1 动态创建 + DoLayout） ----------

    def _show_options(self):
        options = self.dialog.get_options()
        if self.option_panel is not None:
            self.option_panel.SetVisible(True)
        for btn in self._option_buttons:
            if btn is not None:
                btn.SetVisible(False)
        while len(self._option_buttons) < len(options):
            index = len(self._option_buttons)
            btn = CreateButton(self.option_layout, "DynOption{}".format(index))
            if btn is None:
                break
            btn.SetContentSize(Vec2(OPTION_W, OPTION_H))
            btn.SetFontSize(24)
            btn.SetNormalImage(LoadTexture("assets/placeholder/ui/btn_main_normal.png"))
            btn.SetPressedImage(LoadTexture("assets/placeholder/ui/btn_main_pressed.png"))
            btn.OnClicked(lambda b, i=index: self._choose_option(i))
            self._option_buttons.append(btn)
        for i, opt in enumerate(options):
            if i >= len(self._option_buttons):
                break
            btn = self._option_buttons[i]
            btn.SetVisible(True)
            btn.SetText(opt.get("text", ""))
            btn.SetInteractable(True)
        if self.option_layout is not None:
            self.option_layout.DoLayout()

    def _hide_options(self):
        for btn in self._option_buttons:
            if btn is not None:
                btn.SetVisible(False)
        if self.option_panel is not None:
            self.option_panel.SetVisible(False)

    def _choose_option(self, index):
        if self.state != "options":
            return
        target = self.dialog.choose_option(index)
        if target is None:
            return
        self._hide_options()
        self._show_current()

    # ---------- 事件分发 ----------

    def _trigger_battle(self, formation_id):
        """battle_start：保存返回剧情指针（formation.victory），push BattleScene。"""
        formations = DataLoader.get_formations()
        formation = formations.get(formation_id)
        if formation is None:
            self._on_advance_fallback()
            return
        self.game_state.set_battle_return(formation.get("victory"))
        self.game_state.battle_source = "dialog"
        self.state = "event"
        self.main_ref.push("battle", {"formation": formation_id})

    def _trigger_explore(self, scene_id):
        """enter_scene：记录返回节点（当前节点的 next），push ExploreScene。"""
        node = self.dialog.get_node()
        if node is None:
            self._on_advance_fallback()
            return
        next_id = node.get("next")
        if not next_id:
            next_id = self.dialog._resolve_next(node)
        self.game_state.explore_return_node = next_id
        self.state = "event"
        self.main_ref.push("explore", {"scene": scene_id})

    def _trigger_chapter(self, chapter_no):
        """chapter：UISystem.AddUI 叠加 ChapterTransition.cui（独立层 zorder=50），
        淡入淡出后继续；章节图标/装饰 Sprite 上浮动效（策划案 §7.7）；
        顺带更新 GameState.chapter。"""
        node = self.dialog.get_node()
        if node is None:
            return
        self.state = "event"
        # 章节进度（供结局差分与 Staff 使用）；仅接受纯数字参数
        if isinstance(chapter_no, str) and chapter_no.isdigit():
            self.game_state.chapter = int(chapter_no)
        overlay = UISystem.GetInstance().AddUI("assets/ui/ChapterTransition.cui", 50)
        if overlay is None:
            self._on_advance_fallback()
            return
        self._chapter_overlay = overlay
        label = overlay.FindChild("ChapterLabel")
        if label is not None:
            label.SetText(node.get("text", ""))
        # 章节图标动效：从下方上浮 + 透明度淡入（Tweener 位移动画）
        icon = overlay.FindChild("ChapterIcon")
        if icon is not None:
            icon.SetOpacity(0.0)
            icon.SetPosition(Vec3(0, 60, 0))
            self.tweener.to(icon, "position", Vec3(0, 130, 0), dur=0.7,
                            ease="quad_out", delay=0.15)
            self.tweener.to(icon, "opacity", 0.9, dur=0.7, ease="quad_out",
                            delay=0.15)
        overlay.SetOpacity(0.0)
        self.tweener.to(overlay, "opacity", 1.0, dur=0.5, ease="quad_out",
                        on_finish=self._chapter_hold)

    def _chapter_hold(self):
        self.tweener.to(self._chapter_overlay, "opacity", 0.0, dur=0.5,
                        ease="quad_in", delay=1.2, on_finish=self._chapter_done)

    def _chapter_done(self):
        overlay = self._chapter_overlay
        self._chapter_overlay = None
        if overlay is not None:
            UISystem.GetInstance().RemoveUI(overlay)
        self.state = "idle"
        self.dialog.advance()
        self._show_current()

    def _on_advance_fallback(self):
        """事件处理失败兜底：按普通节点推进。"""
        self.state = "idle"
        self.dialog.advance()
        self._show_current()
