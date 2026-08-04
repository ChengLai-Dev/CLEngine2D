# -*- coding: utf-8 -*-
"""战斗场景控制器（BattleScene.cui，最小版）。

- 指令栏：攻击 / 技能 / 防御 / 道具 / 逃跑（B1 禁止逃跑，按钮灰置）
- 行动顺序条（速度降序，同速我方优先；当前行动者高亮）
- 技能/道具子菜单（预置按钮复用；SP 不足灰置；fragment 战斗不可用）
- 单体指令目标选择：点击敌人槽位
- 伤害公式 §6.4 全量（克制/暴击/浮动/保底 1，BattleEngine 结算）
- 命中反馈动画（全部走 Tweener）：施法者前移 → 目标抖动（位移+旋转）→
  伤害数字上飘（暴击金色大字号）→ 技能名放大淡出 → 血条宽度
- 战斗插话条（formations.interludes 按回合显示一次，打字机 + 头像）
- 胜负：敌方全灭 → 经验/升级结算 → push ResultScene；我方全灭 → push GameOverScene
"""

import random

from CLEngine.Math import Vec2, Vec3
from CLEngine.SceneGraph import (
    Scene, SceneManager, UISystem, LoadTexture,
    CreateLabel,
)

from components.Tweener import Tweener
from components.Typewriter import Typewriter
from game import DataLoader
from game.BattleEngine import BattleEngine
from game.GameState import GameState

DIALOG_SPEED = 30.0
DMG_POOL_SIZE = 10

# 我方状态图标映射（StatusIcon{i}_{j} 按优先级显示，4 位最多同时 4 种）
_STATUS_ICONS = [
    ("护盾", "assets/placeholder/ui/icon_status_shield.png"),
    ("封印", "assets/placeholder/ui/icon_status_seal.png"),
    ("降攻", "assets/placeholder/ui/icon_status_atk_dn.png"),
    ("防御up", "assets/placeholder/ui/icon_status_atk_up.png"),
    ("速度up", "assets/placeholder/ui/icon_status_spd_up.png"),
    ("减速", "assets/placeholder/ui/icon_status_spd_dn.png"),
]
STATUS_ICON_SLOTS = 4

# 我方/敌方槽位 UI 前缀
_SLOT_ATTRS = {
    "enemy": {"slot": "EnemySlot{}", "sprite": "EnemySprite{}", "name": "EnemyName{}",
              "hp_fill": "EnemyHpBarFill{}"},
    "player": {"slot": "PlayerSlot{}", "avatar": "Avatar{}", "name": "PlayerName{}",
               "hp_fill": "HpBarFill{}", "sp_fill": "SpBarFill{}"},
}


class BattleScene:
    """战斗控制器。状态机：interlude / player_choice / skill_menu / item_menu /
    target_pick / executing / victory / defeat。"""

    def __init__(self):
        self.name = "battle"
        self.scene = Scene()
        self.scene.LoadUI("assets/ui/BattleScene.cui")
        SceneManager.GetInstance().PushScene(self.scene)
        self.ui_root = UISystem.GetInstance().GetUIRoot()
        self.tweener = Tweener()
        self.engine = None
        self.game_state = GameState()
        self.state = "idle"
        self.current_actor = None
        self._pending_skill = None      # 技能菜单选中的技能（等待目标）
        self._pending_item = None
        self._pending_events = []
        self._interlude = None
        self._dmg_index = 0
        self._tex_cache = {}

        root = self.ui_root
        self.turn_label = root.FindChild("TurnLabel")
        self.bg_battle = root.FindChild("BgBattle")
        self.turn_slots = self._find_slots(root, "TurnSlot{}", 6)
        self.enemy_slots = self._find_slots(root, "EnemySlot{}", 3)
        self.player_slots = self._find_slots(root, "PlayerSlot{}", 3)
        self.enemy_sprite = [s.FindChild("EnemySprite{}".format(i)) for i, s in enumerate(self.enemy_slots)]
        self.enemy_name = [s.FindChild("EnemyName{}".format(i)) for i, s in enumerate(self.enemy_slots)]
        self.enemy_hp_fill = [s.FindChild("EnemyHpBarFill{}".format(i)) for i, s in enumerate(self.enemy_slots)]
        self.player_avatar = [s.FindChild("Avatar{}".format(i)) for i, s in enumerate(self.player_slots)]
        self.player_name = [s.FindChild("PlayerName{}".format(i)) for i, s in enumerate(self.player_slots)]
        self.player_hp_fill = [s.FindChild("HpBarFill{}".format(i)) for i, s in enumerate(self.player_slots)]
        self.player_sp_fill = [s.FindChild("SpBarFill{}".format(i)) for i, s in enumerate(self.player_slots)]
        # 我方状态图标区（StatusIcon{i}_{0..3}）
        self.status_icons = [
            [s.FindChild("StatusIcon{}_".format(i) + str(j)) for j in range(STATUS_ICON_SLOTS)]
            for i, s in enumerate(self.player_slots)
        ]
        # 敌方蓄力警示标签（动态 Label，挂敌方槽位头顶）
        self.charge_labels = [None] * 3

        self.skill_name_label = root.FindChild("SkillNameLabel")
        self.dmg_labels = [root.FindChild("DmgLabel{}".format(i)) for i in range(DMG_POOL_SIZE)]

        self.command_bar = root.FindChild("CommandBar")
        self.btn_attack = root.FindChild("BtnAttack")
        self.btn_skill = root.FindChild("BtnSkill")
        self.btn_guard = root.FindChild("BtnGuard")
        self.btn_item = root.FindChild("BtnItem")
        self.btn_flee = root.FindChild("BtnFlee")

        self.skill_menu = root.FindChild("SkillMenu")
        self.skill_buttons = [root.FindChild("Skill{}".format(i)) for i in range(6)]
        self.skill_desc = root.FindChild("SkillDesc")

        self.item_menu = root.FindChild("ItemMenu")
        self.item_buttons = {
            "herb": root.FindChild("ItemHerb"),
            "dew": root.FindChild("ItemDew"),
            "fragment": root.FindChild("ItemFragment"),
        }
        self.item_back = root.FindChild("ItemMenuBack")

        self.interlude_bar = root.FindChild("InterludeBar")
        self.interlude_avatar = root.FindChild("InterludeAvatar")
        self.interlude_text = root.FindChild("InterludeText")
        self.typewriter = Typewriter(self.interlude_text)

        self.center_zone = root.FindChild("CenterZone")

        # 指令回调
        if self.btn_attack is not None:
            self.btn_attack.OnClicked(lambda b: self._on_attack())
        if self.btn_skill is not None:
            self.btn_skill.OnClicked(lambda b: self._on_open_skill())
        if self.btn_guard is not None:
            self.btn_guard.OnClicked(lambda b: self._on_guard())
        if self.btn_item is not None:
            self.btn_item.OnClicked(lambda b: self._on_open_item())
        if self.btn_flee is not None:
            self.btn_flee.OnClicked(lambda b: self._on_flee())
        for i, btn in enumerate(self.skill_buttons):
            if btn is not None:
                btn.OnClicked(lambda b, idx=i: self._on_skill_chosen(idx))
        for item_id, btn in self.item_buttons.items():
            if btn is not None:
                btn.OnClicked(lambda b, iid=item_id: self._on_item_chosen(iid))
        if self.item_back is not None:
            self.item_back.OnClicked(lambda b: self._close_item_menu())

        # 敌人槽位可点击（选择目标）
        for i, slot in enumerate(self.enemy_slots):
            if slot is not None:
                slot.SetTouchEnabled(True)
                slot.OnTouchStarted(lambda w, pos, idx=i: self._on_enemy_touched(idx))

        # 插话条可点击确认
        if self.interlude_bar is not None:
            self.interlude_bar.SetTouchEnabled(True)
            self.interlude_bar.OnTouchStarted(lambda w, pos: self._confirm_interlude())

    def _find_slots(self, root, pattern, count):
        return [root.FindChild(pattern.format(i)) for i in range(count)]

    # ---------- 场景控制器协议 ----------

    def on_enter(self, params):
        params = params or {}
        formation_id = params.get("formation")
        if formation_id is None:
            formation_id = self.game_state.formation_id
        self.engine = BattleEngine(self.game_state)
        self.engine.start_battle(formation_id)
        self._setup_units_ui()
        self._begin_battle()

    def on_update(self, dt):
        self.tweener.update(dt)
        self.typewriter.update(dt)

    def on_input(self, events):
        for kind, data in events:
            if kind == "confirm":
                if self.state == "interlude":
                    self._confirm_interlude()

    def on_exit(self):
        self.tweener.clear()
        self.state = "idle"
        self._pending_events = []
        self._tex_cache = {}
        self.charge_labels = [None] * 3
        self.engine = None

    # ---------- 战斗构建与 UI 布置 ----------

    def _setup_units_ui(self):
        formation = self.engine.get_formation()
        # 战斗背景（formation.bg 扩展字段；缺省用 .cui 默认）
        if self.bg_battle is not None:
            tex = self._load_tex(formation.get("bg"))
            if tex is not None:
                self.bg_battle.SetTexture(tex)
        # 敌方槽位
        enemies = self.engine.get_alive_enemies()
        for i in range(3):
            if i < len(enemies) and self.enemy_slots[i] is not None:
                self.enemy_slots[i].SetVisible(True)
                unit = enemies[i]
                if self.enemy_sprite[i] is not None:
                    tex = self._load_tex(unit.portrait)
                    if tex is not None:
                        self.enemy_sprite[i].SetTexture(tex)
                if self.enemy_name[i] is not None:
                    self.enemy_name[i].SetText(unit.name)
                self._set_hp_fill_width(self.enemy_hp_fill[i], unit.hp, unit.max_hp, 220.0)
            elif self.enemy_slots[i] is not None:
                self.enemy_slots[i].SetVisible(False)
        # 我方槽位
        players = self.engine.get_player_units()
        for i in range(3):
            if i < len(players) and self.player_slots[i] is not None:
                self.player_slots[i].SetVisible(True)
                unit = players[i]
                player = DataLoader.get_players().get(unit.pid, {})
                if self.player_avatar[i] is not None:
                    tex = self._load_tex(player.get("avatar"))
                    if tex is not None:
                        self.player_avatar[i].SetTexture(tex)
                if self.player_name[i] is not None:
                    self.player_name[i].SetText(unit.name)
                self._set_hp_fill_width(self.player_hp_fill[i], unit.hp, unit.max_hp, 200.0)
                self._set_hp_fill_width(self.player_sp_fill[i], unit.sp, unit.max_sp, 200.0)
            elif self.player_slots[i] is not None:
                self.player_slots[i].SetVisible(False)
        # 行动顺序条
        self._refresh_turn_order()
        # 逃跑按钮：Boss / 禁逃战斗灰置
        if self.btn_flee is not None:
            allowed = formation.get("allow_flee") is True and formation.get("boss") is not True
            self.btn_flee.SetInteractable(allowed)
        # 状态图标 / 蓄力警示初始化
        self._init_charge_labels()
        self._refresh_status_icons()
        self._refresh_charge_labels()

    def _refresh_turn_order(self):
        order = self.engine.get_turn_order()
        for i, slot in enumerate(self.turn_slots):
            if slot is None:
                continue
            if i < len(order):
                slot.SetVisible(True)
                unit = order[i]
                name_label = slot.FindChild("SlotName{}".format(i))
                if name_label is not None:
                    name_label.SetText(unit.name)
                highlight = slot.FindChild("SlotHighlight{}".format(i))
                if highlight is not None:
                    highlight.SetVisible(False)
            else:
                slot.SetVisible(False)
        if self.turn_label is not None:
            self.turn_label.SetText("第 {} 回合".format(self.engine.round))

    def _highlight_actor(self, actor):
        order = self.engine.get_turn_order()
        for i, unit in enumerate(order):
            highlight = self.turn_slots[i].FindChild("SlotHighlight{}".format(i)) \
                if self.turn_slots[i] is not None else None
            if highlight is not None:
                highlight.SetVisible(unit is actor)

    # ---------- 状态图标与蓄力警示 ----------

    def _refresh_status_icons(self):
        """我方状态图标联动：按 status 显隐对应图标（4 位，优先级取前 4 种）。"""
        players = self.engine.get_player_units()
        for i in range(3):
            if i >= len(self.status_icons):
                continue
            icons = [ic for ic in self.status_icons[i] if ic is not None]
            shown = []
            if i < len(players):
                unit = players[i]
                for effect, texture in _STATUS_ICONS:
                    if unit.has_status(effect):
                        shown.append((effect, texture))
            for j, icon in enumerate(icons):
                if j < len(shown):
                    tex = self._load_tex(shown[j][1])
                    if tex is not None:
                        icon.SetTexture(tex)
                    icon.SetVisible(True)
                else:
                    icon.SetVisible(False)

    def _init_charge_labels(self):
        """为敌方槽位创建「蓄力中」动态标签（E1 CreateLabel）。"""
        for i, slot in enumerate(self.enemy_slots):
            if slot is None:
                continue
            if self.charge_labels[i] is not None:
                continue
            label = CreateLabel(slot, "ChargeLabel{}".format(i))
            if label is None:
                continue
            label.SetText("蓄力中")
            label.SetFontSize(22)
            label.SetContentSize(Vec2(120, 30))
            label.SetPosition(Vec3(0, 138, 0))
            label.SetVisible(False)
            self.charge_labels[i] = label

    def _refresh_charge_labels(self):
        """按引擎单位 charging 状态显隐蓄力警示标签。"""
        enemies = [u for u in self.engine.units if u.side == "enemy"]
        for i, label in enumerate(self.charge_labels):
            if label is None:
                continue
            if i < len(enemies) and enemies[i].is_alive() and enemies[i].charging is not None:
                label.SetVisible(True)
            else:
                label.SetVisible(False)

    # ---------- 回合流程 ----------

    def _begin_battle(self):
        il = self.engine.get_interlude(1)
        if il:
            self._show_interlude(il)
            return
        self._start_actor(self.engine.next_actor())

    def _start_actor(self, actor):
        if actor is None:
            self._on_round_end()
            return
        self.current_actor = actor
        self._highlight_actor(actor)
        self._refresh_status_icons()
        self._refresh_charge_labels()
        if actor.side == "player":
            self.state = "player_choice"
            if self.command_bar is not None:
                self.command_bar.SetVisible(True)
        else:
            self.state = "executing"
            events = self.engine.do_enemy_action(actor)
            self._pending_events = events
            self._play_next_event()

    def _on_round_end(self):
        il = self.engine.start_next_round()
        self._refresh_turn_order()
        if il:
            self._show_interlude(il)
            return
        self._start_actor(self.engine.next_actor())

    # ---------- 指令处理 ----------

    def _on_attack(self):
        if self.state != "player_choice":
            return
        self._pending_skill = None
        self.state = "target_pick"
        self._hint("选择目标")

    def _on_open_skill(self):
        if self.state != "player_choice":
            return
        self.state = "skill_menu"
        self._fill_skill_menu()
        if self.skill_menu is not None:
            self.skill_menu.SetVisible(True)

    def _on_guard(self):
        if self.state != "player_choice":
            return
        self._execute_command("guard")

    def _on_open_item(self):
        if self.state != "player_choice":
            return
        self.state = "item_menu"
        self._fill_item_menu()
        if self.item_menu is not None:
            self.item_menu.SetVisible(True)

    def _on_flee(self):
        if self.state != "player_choice":
            return
        self._execute_command("flee")

    def _on_enemy_touched(self, slot_index):
        if self.state != "target_pick":
            return
        enemies = self.engine.get_alive_enemies()
        if slot_index >= len(enemies):
            return
        target = enemies[slot_index]
        if self._pending_skill is not None:
            self._execute_command("skill", skill=self._pending_skill, target=target)
        else:
            self._execute_command("attack", target=target)

    def _on_skill_chosen(self, index):
        if self.state != "skill_menu":
            return
        actor = self.current_actor
        skills = actor.skills
        if not (0 <= index < len(skills)):
            return
        skill = skills[index]
        from game.BattleEngine import SKILL_SP_COST
        cost = SKILL_SP_COST.get(skill.get("id"), 0)
        if actor.has_status("封印"):
            return
        if cost > 0 and actor.has_status("禁SP"):
            return
        if actor.sp < cost:
            return
        # 需要选敌方目标的技能：伤害 / 降防 / 减速（单体）
        needs_target = skill.get("target") == "单体" and \
            skill.get("effect") in ("伤害", "降防", "减速")
        if not needs_target:
            self._close_skill_menu()
            self._execute_command("skill", skill=skill, target=None)
            return
        self._close_skill_menu()
        self._pending_skill = skill
        self.state = "target_pick"
        self._hint("选择目标")

    def _on_item_chosen(self, item_id):
        if self.state != "item_menu":
            return
        items = DataLoader.get_items()
        item = items.get(item_id)
        if item is None:
            return
        if not item.get("usable_in_battle"):
            return
        if self.game_state.get_item_count(item_id) <= 0:
            return
        self._close_item_menu()
        self._execute_command("item", item=item_id)

    # ---------- 指令执行与事件播放 ----------

    def _execute_command(self, kind, skill=None, item=None, target=None):
        self.state = "executing"
        self._hide_menus()
        if self.command_bar is not None:
            self.command_bar.SetVisible(False)
        if kind == "attack":
            events = self.engine.do_attack(self.current_actor, target)
        elif kind == "skill":
            events = self.engine.do_skill(self.current_actor, skill, target)
        elif kind == "guard":
            events = self.engine.do_guard(self.current_actor)
        elif kind == "item":
            events = self.engine.do_item(self.current_actor, item)
        elif kind == "flee":
            events = self.engine.do_flee()
            # 逃跑失败消耗当前行动者的行动（成功则战斗结束）
            if not any(ev.get("type") == "flee_success" for ev in events):
                self.engine.mark_acted(self.current_actor)
        else:
            events = []
        self._pending_events = events
        self._play_next_event()

    def _play_next_event(self):
        if not self._pending_events:
            self._on_events_done()
            return
        ev = self._pending_events.pop(0)
        self._play_event(ev)

    def _play_event(self, ev):
        etype = ev.get("type")
        if etype == "skill_name":
            self._play_skill_name(ev.get("name", ""), self._play_next_event)
        elif etype == "damage":
            self._play_damage(ev)
        elif etype == "death":
            self._play_death(ev)
        elif etype in ("heal", "heal_sp"):
            self._play_heal(ev)
        elif etype == "guard":
            self._hint_text("{}：防御姿态！".format(self._unit_name(ev.get("target"))),
                            self._play_next_event)
        elif etype == "status":
            self._hint_text(self._status_text(ev), lambda: (self._refresh_status_icons(),
                                                            self._play_next_event()))
        elif etype == "charge":
            self._hint_text("{}：开始蓄力！".format(self._unit_name(ev.get("target"))),
                            lambda: (self._refresh_charge_labels(), self._play_next_event()))
        elif etype == "charge_broken":
            self._hint_text("{}：蓄力被打断了！".format(self._unit_name(ev.get("target"))),
                            lambda: (self._refresh_charge_labels(), self._play_next_event()))
        elif etype == "shield_hit":
            self._hint_text("护盾吸收了 {} 点伤害！".format(ev.get("absorbed", 0)),
                            self._play_next_event)
        elif etype == "flee_success":
            self._hint_text("逃跑了！", self._play_next_event)
        elif etype == "flee_fail":
            self._hint_text("没能逃掉！", self._play_next_event)
        elif etype == "fail":
            self._hint_text(self._fail_text(ev), self._play_next_event)
        else:
            self._play_next_event()

    def _on_events_done(self):
        self._refresh_status_icons()
        self._refresh_charge_labels()
        if self.engine.is_over():
            self._on_battle_over()
            return
        self._start_actor(self.engine.next_actor())

    # ---------- 事件动画（全部走 Tweener） ----------

    def _play_skill_name(self, name, on_done):
        label = self.skill_name_label
        if label is None:
            on_done()
            return
        label.SetText(name)
        label.SetVisible(True)
        label.SetOpacity(1.0)
        label.SetScale(Vec3(1.0, 1.0, 1.0))
        self.tweener.to(label, "scale", Vec3(1.6, 1.6, 1.0), dur=0.5, ease="quad_out")
        self.tweener.to(label, "opacity", 0.0, dur=0.5, ease="quad_in",
                        on_finish=lambda: self._reset_skill_label(on_done))

    def _reset_skill_label(self, on_done):
        label = self.skill_name_label
        if label is not None:
            label.SetVisible(False)
            label.SetOpacity(1.0)
            label.SetScale(Vec3(1.0, 1.0, 1.0))
        on_done()

    def _play_damage(self, ev):
        target = ev.get("target")
        self._refresh_unit_bar(target)
        if target is None or target.dead:
            # 目标已死：只出数字（死亡事件随后单独播）
            self._spawn_damage_number(ev)
            self._play_next_event()
            return
        target_spr = self._sprite_of(target)
        actor = ev.get("actor")

        def do_hit():
            self._spawn_damage_number(ev)
            if target_spr is not None:
                orig = target_spr.GetPosition()
                # 抖动：位移去→回、旋转去→回（顺序 tween 链，完成后继续事件）
                self.tweener.to(target_spr, "rotation", 0.15, dur=0.07,
                                on_finish=lambda: self.tweener.to(
                                    target_spr, "rotation", 0.0, dur=0.07))
                self.tweener.to(target_spr, "position",
                                Vec3(orig.x - 12, orig.y, 0), dur=0.07,
                                on_finish=lambda: self.tweener.to(
                                    target_spr, "position", orig, dur=0.07,
                                    on_finish=self._play_next_event))
            else:
                self._play_next_event()

        actor_spr = self._sprite_of(actor)
        if actor_spr is not None:
            orig = actor_spr.GetPosition()
            self.tweener.to(actor_spr, "position", Vec3(orig.x + 70, orig.y, 0),
                            dur=0.18, ease="quad_out",
                            on_finish=lambda: self.tweener.to(
                                actor_spr, "position", orig, dur=0.18,
                                ease="quad_in", on_finish=do_hit))
        else:
            do_hit()

    def _play_death(self, ev):
        target = ev.get("target")
        self._refresh_unit_bar(target)
        spr = self._sprite_of(target)
        if spr is not None:
            orig = spr.GetPosition()
            self.tweener.to(spr, "opacity", 0.0, dur=0.4, ease="quad_in",
                            on_finish=lambda: (spr.SetVisible(False),
                                               self._play_next_event()))
            self.tweener.to(spr, "position", Vec3(orig.x, orig.y - 40, 0),
                            dur=0.4, ease="quad_in")
        else:
            self._play_next_event()

    def _play_heal(self, ev):
        target = ev.get("target")
        self._refresh_unit_bar(target)
        value = ev.get("value", 0)
        label = self._next_dmg_label()
        if label is not None:
            label.SetText("+" + str(value))
            label.SetFontSize(24)
            label.SetVisible(True)
            label.SetOpacity(1.0)
            pos = self._label_pos(self._sprite_of(target))
            label.SetPosition(pos)
            self.tweener.to(label, "position", Vec3(pos.x, pos.y + 50, pos.z),
                            dur=0.6, ease="quad_out")
            self.tweener.to(label, "opacity", 0.0, dur=0.6, delay=0.2,
                            on_finish=lambda: label.SetVisible(False))
        self._play_next_event()

    def _spawn_damage_number(self, ev):
        target = ev.get("target")
        label = self._next_dmg_label()
        if label is None:
            return
        crit = ev.get("crit", False)
        label.SetText(str(ev.get("value", 0)))
        label.SetFontSize(34 if crit else 26)
        label.SetVisible(True)
        label.SetOpacity(1.0)
        pos = self._label_pos(self._sprite_of(target))
        jitter = random.uniform(-24.0, 24.0)
        pos = Vec3(pos.x + jitter, pos.y + 20.0, pos.z)
        label.SetPosition(pos)
        self.tweener.to(label, "position", Vec3(pos.x, pos.y + 46.0, pos.z),
                        dur=0.6, ease="quad_out")
        self.tweener.to(label, "opacity", 0.0, dur=0.6, delay=0.15,
                        on_finish=lambda: label.SetVisible(False))

    def _next_dmg_label(self):
        labels = [l for l in self.dmg_labels if l is not None]
        if not labels:
            return None
        label = labels[self._dmg_index % len(labels)]
        self._dmg_index += 1
        return label

    def _sprite_of(self, unit):
        if unit is None:
            return None
        if unit.side == "enemy":
            index = self._enemy_index(unit)
            if index is not None:
                return self.enemy_sprite[index]
        return None

    def _enemy_index(self, unit):
        enemies = [u for u in self.engine.units if u.side == "enemy"]
        for i, u in enumerate(enemies):
            if u is unit:
                return i
        return None

    def _label_pos(self, node):
        """伤害数字挂 CenterZone：目标世界坐标换算成 CenterZone 局部坐标。"""
        base = self.center_zone.GetPosition() if self.center_zone is not None else Vec3(0, 0, 0)
        if node is None:
            return Vec3(0, 40, 0)
        world = self._world_pos(node)
        return Vec3(world.x - base.x, world.y - base.y, 0)

    def _world_pos(self, node):
        pos = node.GetPosition()
        parent = node.GetParent()
        while parent is not None and parent is not self.ui_root:
            p = parent.GetPosition()
            pos = Vec3(pos.x + p.x, pos.y + p.y, pos.z + p.z)
            parent = parent.GetParent()
        return pos

    # ---------- 血条 ----------

    def _set_hp_fill_width(self, fill, value, max_value, full_width):
        if fill is None:
            return
        ratio = value / max_value if max_value > 0 else 0.0
        self.tweener.to(fill, "width", full_width * ratio, dur=0.25)

    def _refresh_unit_bar(self, unit):
        if unit is None:
            return
        if unit.side == "enemy":
            index = self._enemy_index(unit)
            if index is not None:
                self._set_hp_fill_width(self.enemy_hp_fill[index], unit.hp,
                                        unit.max_hp, 220.0)
        else:
            for i, u in enumerate(self.engine.get_player_units()):
                if u is unit:
                    self._set_hp_fill_width(self.player_hp_fill[i], unit.hp,
                                            unit.max_hp, 200.0)
                    self._set_hp_fill_width(self.player_sp_fill[i], unit.sp,
                                            unit.max_sp, 200.0)

    # ---------- 子菜单 ----------

    def _fill_skill_menu(self):
        actor = self.current_actor
        if actor is None:
            return
        from game.BattleEngine import SKILL_SP_COST
        sealed = actor.has_status("封印")
        sp_blocked = actor.has_status("禁SP")
        for i, btn in enumerate(self.skill_buttons):
            if btn is None:
                continue
            if i < len(actor.skills):
                skill = actor.skills[i]
                cost = SKILL_SP_COST.get(skill.get("id"), 0)
                btn.SetVisible(True)
                btn.SetText("{}（SP {}）".format(skill.get("name", ""), cost))
                usable = not sealed and not (sp_blocked and cost > 0) and actor.sp >= cost
                btn.SetInteractable(usable)
            else:
                btn.SetVisible(False)
        if self.skill_desc is not None:
            if sealed:
                self.skill_desc.SetText("已被封印：无法使用技能，只能用攻击/防御/道具。")
            elif sp_blocked:
                self.skill_desc.SetText("SP 被禁：本回合无法消耗 SP 施放技能。")
            else:
                self.skill_desc.SetText("技能说明：点击技能查看。SP 不足时按钮置灰。")

    def _fill_item_menu(self):
        items = DataLoader.get_items()
        for item_id, btn in self.item_buttons.items():
            if btn is None:
                continue
            item = items.get(item_id)
            if item is None:
                btn.SetVisible(False)
                continue
            btn.SetVisible(True)
            count = self.game_state.get_item_count(item_id)
            btn.SetText("{} ×{}".format(item.get("name", item_id), count))
            btn.SetInteractable(item.get("usable_in_battle") is True and count > 0)

    def _close_skill_menu(self):
        if self.skill_menu is not None:
            self.skill_menu.SetVisible(False)

    def _close_item_menu(self):
        if self.item_menu is not None:
            self.item_menu.SetVisible(False)

    def _hide_menus(self):
        self._close_skill_menu()
        self._close_item_menu()

    # ---------- 插话 ----------

    def _show_interlude(self, il):
        self._interlude = il
        self.state = "interlude"
        if self.interlude_bar is not None:
            self.interlude_bar.SetVisible(True)
        portrait = DataLoader.get_characters().get("portraits", {}).get(il.get("portrait"))
        if self.interlude_avatar is not None and portrait is not None:
            tex = self._load_tex(portrait.get("avatar"))
            if tex is not None:
                self.interlude_avatar.SetTexture(tex)
        if self.interlude_text is not None:
            self.typewriter.start(il.get("text", ""), DIALOG_SPEED)

    def _confirm_interlude(self):
        if self.state != "interlude":
            return
        self._interlude = None
        self.state = "idle"
        if self.interlude_bar is not None:
            self.interlude_bar.SetVisible(False)
        self._start_actor(self.engine.next_actor())

    # ---------- 提示 ----------

    def _hint(self, text):
        if self.skill_name_label is not None:
            self.skill_name_label.SetText(text)
            self.skill_name_label.SetVisible(True)

    def _hint_text(self, text, on_done):
        label = self.skill_name_label
        if label is None:
            on_done()
            return
        label.SetText(text)
        label.SetVisible(True)
        label.SetOpacity(1.0)
        self.tweener.to(label, "opacity", 0.0, dur=0.7, delay=0.5,
                        on_finish=lambda: self._reset_skill_label(on_done))

    @staticmethod
    def _unit_name(unit):
        return unit.name if unit is not None else ""

    @staticmethod
    def _status_text(ev):
        effect = ev.get("effect", "")
        target = ev.get("target")
        name = BattleScene._unit_name(target)
        names = {"防御up": "防御提升了", "速度up": "速度提升了",
                 "降防": "防御下降了", "减速": "速度下降了",
                 "降攻": "攻击下降了", "封印": "被封印了，无法使用技能",
                 "护盾": "展开了护盾，可吸收伤害", "禁SP": "SP 被禁用了"}
        return "{}：{}！".format(name, names.get(effect, effect))

    @staticmethod
    def _fail_text(ev):
        reason = ev.get("reason", "")
        return {"sp": "SP 不足！", "item": "道具不存在！", "count": "道具数量不足！",
                "unusable": "该道具无法在战斗中使用！",
                "no_flee": "这场战斗无法逃跑！",
                "sealed": "被封印了，无法使用技能！",
                "sp_blocked": "SP 被禁用了！",
                "target": "目标无效！"}.get(reason, "失败了！")

    # ---------- 胜负 ----------

    def _on_battle_over(self):
        self._sync_party()
        over = self.engine.over
        if over == "victory":
            self.state = "victory"
            formation = self.engine.get_formation()
            # 记录已击败编成（ExploreScene 据此隐藏明雷/事件触发）
            self.game_state.set_flag("visited_" + self.engine.formation_id, 1)
            exp = formation.get("exp_reward", 0)
            level_ups = self.engine.gain_exp(exp)
            self.game_state.set_battle_result({"exp": exp, "level_ups": level_ups})
            self.main_ref.push("result", {})
        elif over == "defeat":
            self.state = "defeat"
            self.main_ref.push("gameover", {})
        elif over == "flee":
            self.main_ref.pop()
        else:
            self.state = "idle"

    def _sync_party(self):
        """把战斗结束时的 HP/SP 写回 GameState（升级在结算场景前完成）。"""
        for unit in self.engine.get_player_units():
            member = self.game_state.get_member(unit.pid)
            if member is not None:
                member["hp"] = unit.hp
                member["sp"] = unit.sp

    # ---------- 工具 ----------

    def _load_tex(self, path):
        if not path:
            return None
        if path in self._tex_cache:
            return self._tex_cache[path]
        tex = LoadTexture(path)
        self._tex_cache[path] = tex
        return tex
