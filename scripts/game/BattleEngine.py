# -*- coding: utf-8 -*-
"""战斗纯逻辑（方案文档 §3.1 / 策划案 §6.2~6.4，阶段 4 完整版）。

职责：
- 战斗实例构建（我方 = GameState 队伍、敌方 = formations.enemies 展开）
- 行动顺序（速度降序，同速我方优先）
- 指令结算：攻击 / 技能 / 防御 / 道具 / 逃跑
- 伤害公式全量（§6.4：物理/魔法、克制 1.5/0.7、浮动 0.9~1.1、暴击 5%×1.5、保底 1、防御减半）
- 状态效果全量：降攻/降防/防御up/速度up/减速/封印/禁SP/护盾（护盾先于防御判定）
- 蓄力与打断（终焉之书：蓄力 1 回合 → 下回合行动释放全体 2.0 倍；受击即打断）
- 敌人 AI 完整版：simple（chance 加权）/ tank（防御up/高倍率）/ caster（封印优先）/ boss（轮换加权）
- 状态持续回合结算（单位行动时 decrement）
- 升级成长（§6.4 固定成长）；Boss 战保底升 1 级；技能解锁按等级
- 回合插话触发、胜负判定、经验与升级

不依赖引擎 UI；结算结果以事件列表形式返回，由场景层播放动画。
"""

import random

from game import DataLoader

# ---------- 新增设计数值（策划案/数据文件未定，阶段 3/4 定稿，交付说明记录） ----------

# 技能 SP 消耗表（players.json skills 无 sp_cost 字段；data/ 只读，故集中于此）
SKILL_SP_COST = {
    "light_slash": 6, "light_burst": 10, "mend": 8,
    "water_shard": 6, "tidal_wave": 10, "chill": 8,
    "blaze_slash": 6, "armor_break": 10, "steel_body": 8,
    "thunder_crash": 8, "haste": 6, "barrier": 8,
}

# 升级曲线：每级固定 60 经验（阶段 4 定稿：与 B2~B8 exp_reward 配套，
# 逐场对齐 §6.6 推荐等级：Lv1→2→2→4→4→6→8(保底)→11(保底)）
EXP_PER_LEVEL = 60

# 逃跑成功率（策划案 §6.3：非 Boss 战 80%）
FLEE_RATE = 0.8

# 元素克制（策划案 §6.4：单向循环 + 光暗互克 + 无属性 1.0）

_ADVANTAGE = {"火": "木", "木": "雷", "雷": "水", "水": "火"}


def element_multiplier(attacker_elem, target_elem):
    """克制系数：克制 1.5，被克 0.7，光↔暗互克，无属性 1.0。"""
    if attacker_elem == "无" or target_elem == "无":
        return 1.0
    if _ADVANTAGE.get(attacker_elem) == target_elem:
        return 1.5
    if _ADVANTAGE.get(target_elem) == attacker_elem:
        return 0.7
    if (attacker_elem == "光" and target_elem == "暗") or \
            (attacker_elem == "暗" and target_elem == "光"):
        return 1.5
    return 1.0


def calc_physical(atk, multiplier, target_def):
    """物理伤害基数：(攻击 × 倍率 − 目标防御 × 0.6)。"""
    return atk * multiplier - target_def * 0.6


def calc_magical(mag, multiplier, target_mdef):
    """魔法伤害基数：(魔力 × 倍率 − 目标魔防 × 0.5)。"""
    return mag * multiplier - target_mdef * 0.5


# 状态效果数值定义（effect 值域与数据文件一致）
_STATUS_RATE = {
    "降攻": 0.2,       # 攻击 -20%
    "降防": 0.2,       # 防御 -20%
    "防御up": 0.5,     # 防御 +50%
    "速度up": 0.2,     # 速度 +20%
    "减速": 0.2,       # 速度 -20%
}

# 护盾：吸收值型状态（无持续回合，吸收完即消失，duration 占位极大值）
SHIELD_DURATION = 999


# ---------- 战斗单位 ----------

class BattleUnit:
    """战斗实例中的单个单位（我方或敌方）。"""

    def __init__(self, side, uid, name, element, stats, skills, portrait=None, ai="simple"):
        self.side = side          # "player" / "enemy"
        self.uid = uid            # 实例唯一 id（如 p0 / e0）
        self.id = uid             # 兼容：单位逻辑 id
        self.pid = None           # 我方角色 id（players.json）；敌方为 None
        self.name = name
        self.element = element
        self.max_hp = stats.get("hp", 1)
        self.max_sp = stats.get("sp", 0)
        self.hp = self.max_hp
        self.sp = self.max_sp
        self.atk = stats.get("atk", 0)
        self.defence = stats.get("def", 0)
        self.mag = stats.get("mag", 0)
        self.mdef = stats.get("mdef", 0)
        self.spd = stats.get("spd", 0)
        self.skills = list(skills)
        self.portrait = portrait
        self.ai = ai              # 敌人 AI 类型（simple/tank/caster/boss）
        self.guard = False
        self.dead = False
        self.status = {}          # effect -> {"value": int, "duration": int}
        self.charging = None      # 蓄力技能 dict；None = 未蓄力
        self._last_skill = None   # Boss 轮换：上次使用的技能 id

    def is_alive(self):
        return not self.dead

    # ---------- 状态效果 ----------

    def has_status(self, effect):
        return effect in self.status

    def effective_atk(self):
        """攻击（降攻 -20% 修正）。"""
        if self.has_status("降攻"):
            return int(self.atk * (1.0 - _STATUS_RATE["降攻"]))
        return self.atk

    def effective_def(self):
        """防御（防御up +50% 与降防 -20% 叠加修正）。"""
        mult = 1.0
        if self.has_status("防御up"):
            mult *= (1.0 + _STATUS_RATE["防御up"])
        if self.has_status("降防"):
            mult *= (1.0 - _STATUS_RATE["降防"])
        return int(self.defence * mult)

    def effective_spd(self):
        """速度（速度up/减速 ±20% 修正）。"""
        mult = 1.0
        if self.has_status("速度up"):
            mult *= (1.0 + _STATUS_RATE["速度up"])
        if self.has_status("减速"):
            mult *= (1.0 - _STATUS_RATE["减速"])
        return int(self.spd * mult)

    # ---------- 生命/资源 ----------

    def take_damage(self, value):
        if value < 0:
            value = 0
        self.hp -= value
        if self.hp <= 0:
            self.hp = 0
            self.dead = True
            self.status = {}
            self.charging = None

    def heal_hp(self, value):
        self.hp = min(self.max_hp, self.hp + value)

    def heal_sp(self, value):
        self.sp = min(self.max_sp, self.sp + value)

    def spend_sp(self, value):
        if self.sp < value:
            return False
        self.sp -= value
        return True


# ---------- 战斗引擎 ----------

class BattleEngine:

    def __init__(self, game_state):
        self.game_state = game_state
        self.formation_id = None
        self.formation = None
        self.units = []          # 全部单位
        self.round = 1
        self.over = None         # None / "victory" / "defeat" / "flee"
        self.events = []         # 待播放事件队列（场景层消费）
        self._acted = []         # 本回合已行动单位
        self._interlude_shown = set()   # 已显示过的插话（round, index）

    # ---------- 战斗构建 ----------

    def start_battle(self, formation_id):
        """按编成构建战斗；保存战前队伍快照供 GameOver 重试。"""
        formations = DataLoader.get_formations()
        formation = formations.get(formation_id)
        if formation is None:
            raise ValueError("编成不存在: " + str(formation_id))
        self.formation_id = formation_id
        self.formation = formation
        self.units = []
        self.round = 1
        self.over = None
        self.events = []
        self._acted = []
        self._interlude_shown = set()

        self.game_state.formation_id = formation_id
        self.game_state.snapshot_party()

        enemies = DataLoader.get_enemies()
        index = 0
        for entry in formation.get("enemies", []):
            enemy = enemies.get(entry.get("enemy_id"))
            if enemy is None:
                continue
            for _ in range(entry.get("count", 1)):
                uid = "e{}".format(index)
                self.units.append(BattleUnit(
                    "enemy", uid, enemy.get("name", entry.get("enemy_id")),
                    enemy.get("element", "无"),
                    {"hp": enemy.get("hp", 1), "sp": 0,
                     "atk": enemy.get("atk", 0), "def": enemy.get("def", 0),
                     "mag": enemy.get("mag", 0), "mdef": enemy.get("mdef", 0),
                     "spd": enemy.get("spd", 0)},
                    enemy.get("skills", []),
                    portrait=enemy.get("portrait"),
                    ai=enemy.get("ai", "simple")))
                index += 1

        for pid in self.game_state.party:
            member = self.game_state.get_member(pid)
            stats = self.game_state.get_member_stats(pid)
            p = DataLoader.get_players().get(pid, {})
            uid = "p{}".format(len([u for u in self.units if u.side == "player"]))
            unit = BattleUnit(
                "player", uid, p.get("name", pid),
                p.get("element", "无"),
                {"hp": stats.get("hp", 1), "sp": stats.get("sp", 0),
                 "atk": stats.get("atk", 0), "def": stats.get("def", 0),
                 "mag": stats.get("mag", 0), "mdef": stats.get("mdef", 0),
                 "spd": stats.get("spd", 0)},
                self.game_state.get_available_skills(pid),
                portrait=p.get("portrait"))
            unit.hp = member.get("hp", unit.max_hp)
            unit.sp = member.get("sp", unit.max_sp)
            unit.pid = pid
            self.units.append(unit)
        return self

    # ---------- 查询 ----------

    def get_formation(self):
        return self.formation

    def get_alive_enemies(self):
        return [u for u in self.units if u.side == "enemy" and u.is_alive()]

    def get_alive_players(self):
        return [u for u in self.units if u.side == "player" and u.is_alive()]

    def get_player_units(self):
        return [u for u in self.units if u.side == "player"]

    def get_turn_order(self):
        """行动顺序：速度降序（含状态修正）；同速我方优先。"""
        alive = [u for u in self.units if u.is_alive()]
        alive.sort(key=lambda u: (-u.effective_spd(), 0 if u.side == "player" else 1))
        return alive

    def next_actor(self):
        """本回合下一个未行动单位；全部行动完返回 None。"""
        for u in self.get_turn_order():
            if u not in self._acted:
                return u
        return None

    def start_next_round(self):
        """回合结束推进；返回插话 dict（该回合开始时显示）或 None。"""
        self._acted = []
        self.round += 1
        for u in self.units:
            u.guard = False
        return self.get_interlude(self.round)

    def get_interlude(self, round_number):
        """第 round_number 回合开始时的插话（formations.interludes）；无则 None。"""
        for il in self.formation.get("interludes", []):
            if il.get("round") == round_number and il.get("text"):
                return il
        return None

    def is_over(self):
        """胜负判定：敌方全灭=胜利，我方全灭=失败。"""
        if not self.get_alive_enemies():
            self.over = "victory"
        elif not self.get_alive_players():
            self.over = "defeat"
        return self.over

    def mark_acted(self, actor):
        """标记单位本回合已行动（next_actor 轮转；幂等）。"""
        if actor is not None and actor not in self._acted:
            self._acted.append(actor)

    # ---------- 状态效果 ----------

    def _add_status(self, target, effect, value, duration):
        """为目标附加状态（覆盖同名状态，重置持续回合）；护盾为吸收值型。"""
        if target is None or target.dead or duration <= 0:
            return
        if effect == "护盾":
            target.status["护盾"] = {"value": value, "duration": SHIELD_DURATION}
            return
        target.status[effect] = {"value": value, "duration": duration}

    def _tick_status(self, unit):
        """状态持续回合结算：单位行动时 decrement，归零移除。"""
        for effect in list(unit.status.keys()):
            entry = unit.status[effect]
            if effect == "护盾":
                continue          # 吸收值型状态不按回合衰减
            entry["duration"] -= 1
            if entry["duration"] <= 0:
                del unit.status[effect]

    def is_sealed(self, unit):
        """封印：禁技能（普攻/防御/道具仍可用）。"""
        return unit.has_status("封印")

    def is_sp_blocked(self, unit):
        """禁SP：1 回合禁 SP 消耗。"""
        return unit.has_status("禁SP")

    # ---------- 指令结算（返回事件列表，供场景层播放） ----------

    def do_attack(self, actor, target):
        """普通攻击：物理，倍率 1.0。"""
        actor.guard = False
        self.mark_acted(actor)
        self._tick_status(actor)
        return self._apply_damage_skill(actor, target, {
            "name": "攻击", "type": "物理", "target": "单体",
            "multiplier": 1.0, "element": actor.element,
        })

    def do_skill(self, actor, skill, target):
        """技能施放；技能伤害/回复/状态效果，扣除 SP。封印/禁SP 检查。"""
        actor.guard = False
        sealed = self.is_sealed(actor)
        sp_blocked = self.is_sp_blocked(actor)
        self._tick_status(actor)
        if sealed:
            self.events.append({"type": "fail", "reason": "sealed", "actor": actor})
            return self._drain_events()
        cost = SKILL_SP_COST.get(skill.get("id"), 0)
        if cost > 0 and sp_blocked:
            self.events.append({"type": "fail", "reason": "sp_blocked", "actor": actor})
            return self._drain_events()
        if not actor.spend_sp(cost):
            self.events.append({"type": "fail", "reason": "sp", "actor": actor})
            return self._drain_events()
        self.mark_acted(actor)

        effect = skill.get("effect")
        if effect == "回复":
            self.events.append({"type": "skill_name", "name": skill.get("name", "")})
            self.events.append({"type": "heal", "target": actor, "value": skill.get("effect_value", 0)})
        elif effect in ("防御up", "速度up"):
            # 自身增益
            self.events.append({"type": "skill_name", "name": skill.get("name", "")})
            self._add_status(actor, effect, skill.get("effect_value", 0),
                             skill.get("duration", 0))
            self.events.append({"type": "status", "target": actor, "effect": effect,
                                "value": skill.get("effect_value", 0),
                                "duration": skill.get("duration", 0)})
        elif effect in ("降防", "减速"):
            # 敌方单体减益（须先选目标）
            if target is None or target.dead:
                self.events.append({"type": "fail", "reason": "target", "actor": actor})
                return self._drain_events()
            self.events.append({"type": "skill_name", "name": skill.get("name", "")})
            self._add_status(target, effect, skill.get("effect_value", 0),
                             skill.get("duration", 0))
            self.events.append({"type": "status", "target": target, "effect": effect,
                                "value": skill.get("effect_value", 0),
                                "duration": skill.get("duration", 0)})
        elif effect == "护盾":
            # 自身护盾（吸收 value 点伤害，先于防御判定）
            self.events.append({"type": "skill_name", "name": skill.get("name", "")})
            self._add_status(actor, "护盾", skill.get("effect_value", 0), 1)
            self.events.append({"type": "status", "target": actor, "effect": "护盾",
                                "value": skill.get("effect_value", 0), "duration": 0})
        else:
            return self._apply_damage_skill(actor, target, skill)
        return self._drain_events()

    def do_guard(self, actor):
        """防御：本回合受击伤害减半。"""
        actor.guard = True
        self.mark_acted(actor)
        self._tick_status(actor)
        self.events.append({"type": "guard", "target": actor})
        return self._drain_events()

    def do_item(self, actor, item_id):
        """道具：作用目标为当前行动者自身。"""
        actor.guard = False
        self._tick_status(actor)
        items = DataLoader.get_items()
        item = items.get(item_id)
        if item is None:
            self.events.append({"type": "fail", "reason": "item", "actor": actor})
            return self._drain_events()
        if not self.game_state.consume_item(item_id):
            self.events.append({"type": "fail", "reason": "count", "actor": actor})
            return self._drain_events()
        self.mark_acted(actor)
        if item.get("effect_type") == "heal_hp":
            self.events.append({"type": "heal", "target": actor, "value": item.get("effect_value", 0)})
        elif item.get("effect_type") == "heal_sp":
            self.events.append({"type": "heal_sp", "target": actor, "value": item.get("effect_value", 0)})
        else:
            self.events.append({"type": "fail", "reason": "unusable", "actor": actor})
        return self._drain_events()

    def do_flee(self):
        """逃跑：非 Boss 战 80% 成功；成功则战斗结束。"""
        formation = self.formation
        if formation.get("allow_flee") is False or formation.get("boss") is True:
            self.events.append({"type": "fail", "reason": "no_flee"})
            return self._drain_events()
        if random.random() < FLEE_RATE:
            self.over = "flee"
            self.events.append({"type": "flee_success"})
        else:
            self.events.append({"type": "flee_fail"})
        return self._drain_events()

    # ---------- 敌人 AI 完整版 ----------

    def do_enemy_action(self, actor):
        """敌人 AI：封印中只会普攻；蓄力中释放；按 ai 类型选技能。"""
        actor.guard = False
        self.mark_acted(actor)
        sealed = self.is_sealed(actor)
        self._tick_status(actor)
        if sealed:
            return self.do_attack(actor, self._pick_random_target())
        if actor.charging is not None:
            return self.release_charge(actor)
        skills = actor.skills
        if not skills:
            return self.do_attack(actor, self._pick_random_target())
        ai = getattr(actor, "ai", "simple")
        if ai == "tank":
            skill = self._ai_tank(actor)
        elif ai == "caster":
            skill = self._ai_caster(actor)
        elif ai == "boss":
            skill = self._ai_boss(actor)
        else:
            skill = self._ai_simple(actor)
        if skill is None:
            return self.do_attack(actor, self._pick_random_target())
        return self._perform_enemy_skill(actor, skill)

    def _ai_simple(self, actor):
        """simple：chance 加权随机（阶段 3 现行为保留）。"""
        skills = actor.skills
        weights = [max(s.get("chance", 1.0), 0.0) for s in skills]
        return random.choices(skills, weights=weights, k=1)[0]

    def _ai_tank(self, actor):
        """tank：未 buff 时优先防御up；否则高倍率伤害。"""
        if not actor.has_status("防御up"):
            for s in actor.skills:
                if s.get("effect") == "防御up":
                    return s
        damages = [s for s in actor.skills if s.get("effect") == "伤害"]
        if damages:
            return max(damages, key=lambda s: s.get("multiplier", 0))
        return self._ai_simple(actor)

    def _ai_caster(self, actor):
        """caster：封印优先于伤害（存在未封印目标时）；否则加权选伤害。"""
        seal = None
        for s in actor.skills:
            if s.get("effect") == "封印":
                seal = s
        if seal is not None:
            open_targets = [t for t in self.get_alive_players() if not self.is_sealed(t)]
            if open_targets:
                return seal
        damages = [s for s in actor.skills if s.get("effect") == "伤害"]
        if damages:
            weights = [max(s.get("chance", 1.0), 0.0) for s in damages]
            return random.choices(damages, weights=weights, k=1)[0]
        return self._ai_simple(actor)

    def _ai_boss(self, actor):
        """boss：chance 加权随机，但避开上次技能（轮换），蓄力照常使用。"""
        skills = actor.skills
        pool = [s for s in skills if s.get("id") != actor._last_skill]
        if not pool:
            pool = skills
        weights = [max(s.get("chance", 1.0), 0.0) for s in pool]
        pick = random.choices(pool, weights=weights, k=1)[0]
        actor._last_skill = pick.get("id")
        return pick

    def _perform_enemy_skill(self, actor, skill):
        """按技能 effect 执行敌人动作（状态类不进伤害流程）。"""
        effect = skill.get("effect")
        if effect == "防御up":
            self.events.append({"type": "skill_name", "name": skill.get("name", "")})
            self._add_status(actor, "防御up", skill.get("effect_value", 0),
                             skill.get("duration", 0))
            self.events.append({"type": "status", "target": actor, "effect": "防御up",
                                "value": skill.get("effect_value", 0),
                                "duration": skill.get("duration", 0)})
            return self._drain_events()
        if effect == "封印":
            open_targets = [t for t in self.get_alive_players() if not self.is_sealed(t)]
            target = random.choice(open_targets) if open_targets else self._pick_random_target()
            self.events.append({"type": "skill_name", "name": skill.get("name", "")})
            self._add_status(target, "封印", 0, skill.get("duration", 0))
            self.events.append({"type": "status", "target": target, "effect": "封印",
                                "value": 0, "duration": skill.get("duration", 0)})
            return self._drain_events()
        if effect == "蓄力":
            actor.charging = skill
            self.events.append({"type": "charge", "target": actor,
                                "skill": skill.get("name", "")})
            return self._drain_events()
        target = self._pick_random_target()
        return self._apply_damage_skill(actor, target, skill)

    def release_charge(self, actor):
        """蓄力单位行动时释放蓄力技能（全体倍率按技能结算）。"""
        skill = actor.charging
        actor.charging = None
        if skill is None:
            return []
        return self._apply_damage_skill(actor, None, skill)

    # ---------- 伤害结算核心（§6.4 全量 + 护盾/打断/附加效果） ----------

    def _apply_damage_skill(self, actor, target, skill):
        """按技能结算伤害：克制/浮动/暴击/保底 1/护盾/防御减半/蓄力打断/死亡。"""
        if skill.get("target") != "全体" and (target is None or not target.is_alive()):
            return []
        skill_name = skill.get("name", "攻击")
        multiplier = skill.get("multiplier", 1.0)
        self.events.append({"type": "skill_name", "name": skill_name})

        targets = [target]
        if skill.get("target") == "全体":
            if actor.side == "player":
                targets = self.get_alive_enemies()
            else:
                targets = self.get_alive_players()
        if not targets:
            return self._drain_events()

        is_magic = skill.get("type") == "魔法"
        # 暴击率：5% 仅攻击技能（伤害型）；敌人技能 crit_bonus 加算
        crit_rate = 0.05
        if actor.side == "enemy":
            crit_rate += skill.get("crit_bonus", 0.0)

        for t in targets:
            if not t.is_alive():
                continue
            if is_magic:
                raw = calc_magical(actor.mag, multiplier, t.mdef)
            else:
                raw = calc_physical(actor.effective_atk(), multiplier, t.effective_def())
            elem = element_multiplier(actor.element, t.element)
            crit = random.random() < crit_rate
            dmg = raw * elem * random.uniform(0.9, 1.1)
            if crit:
                dmg *= 1.5
            dmg = int(max(dmg, 1.0))
            # 护盾吸收（先于防御判定；护盾吸收的是防御判定前的伤害）
            if t.has_status("护盾"):
                shield = t.status["护盾"]
                absorbed = int(min(shield.get("value", 0), dmg))
                if absorbed > 0:
                    shield["value"] -= absorbed
                    dmg -= absorbed
                    self.events.append({"type": "shield_hit", "target": t,
                                        "absorbed": absorbed})
                if shield.get("value", 0) <= 0:
                    del t.status["护盾"]
            if t.guard:
                dmg = int(max(dmg / 2.0, 1.0))
            # 蓄力打断：命中蓄力中目标（且未致死）即打断
            if t.charging is not None and not t.dead:
                t.charging = None
                self.events.append({"type": "charge_broken", "target": t})
            self.events.append({"type": "damage", "actor": actor, "target": t,
                                "value": dmg, "crit": crit, "skill": skill_name})
            t.take_damage(dmg)
            if t.dead:
                self.events.append({"type": "death", "target": t})
            # 附加效果（伤害型技能的 extra_effect：降攻/降防/禁SP 等）
            extra = skill.get("extra_effect")
            if extra and not t.dead:
                self._add_status(t, extra, skill.get("extra_value", 0),
                                 skill.get("extra_duration", 0))
                self.events.append({"type": "status", "target": t, "effect": extra,
                                    "value": skill.get("extra_value", 0),
                                    "duration": skill.get("extra_duration", 0)})
        return self._drain_events()

    def _pick_random_target(self):
        alive = self.get_alive_players()
        if not alive:
            return None
        return random.choice(alive)

    def _drain_events(self):
        """取走并清空当前事件队列（场景层播放用）。"""
        result = list(self.events)
        self.events = []
        return result

    # ---------- 经验与升级 ----------

    def exp_to_next(self, level):
        """升到下一级所需经验：每级固定 60（阶段 4 定稿）。"""
        return EXP_PER_LEVEL

    def _apply_level_up(self, pid):
        """单个成员升级结算（经验足够则逐级升级）；返回升级明细或 None。"""
        member = self.game_state.get_member(pid)
        if member is None:
            return None
        p = DataLoader.get_players().get(pid, {})
        max_level = p.get("max_level", 12)
        old_level = member["level"]
        gained = 0
        while member["level"] < max_level and member["exp"] >= self.exp_to_next(member["level"]):
            member["exp"] -= self.exp_to_next(member["level"])
            member["level"] += 1
            gained += 1
        if gained <= 0:
            return None
        new_level = member["level"]
        old_stats = self._stats_at_level(p, old_level)
        new_stats = self._stats_at_level(p, new_level)
        member["hp"] = new_stats["hp"]
        member["sp"] = new_stats["sp"]
        return {
            "pid": pid,
            "name": p.get("name", pid),
            "old_level": old_level,
            "new_level": new_level,
            "gains": {k: new_stats[k] - old_stats[k] for k in ("hp", "sp", "atk", "def", "mag", "mdef", "spd")},
        }

    def gain_exp(self, exp):
        """全队获得经验并结算升级；Boss 战胜利后对未满级成员补足经验强制升 1 级
        （§6.6「B8 之前保证玩家等级 ≥ 推荐等级」：B7→Lv8、B8→Lv11）。
        返回升级明细列表。"""
        level_ups = []
        for pid in self.game_state.party:
            member = self.game_state.get_member(pid)
            if member is None:
                continue
            member["exp"] += exp
            up = self._apply_level_up(pid)
            if up is not None:
                level_ups.append(up)
        if self.formation is not None and self.formation.get("boss") is True:
            # Boss 保底：无论自然升级与否，未满级成员补足经验再升 1 级
            for pid in self.game_state.party:
                member = self.game_state.get_member(pid)
                if member is None:
                    continue
                p = DataLoader.get_players().get(pid, {})
                if member["level"] >= p.get("max_level", 12):
                    continue
                needed = self.exp_to_next(member["level"]) - member["exp"]
                member["exp"] += max(needed, 0)
                up = self._apply_level_up(pid)
                if up is not None:
                    level_ups.append(up)
        return level_ups

    @staticmethod
    def _stats_at_level(player, level):
        stats = {}
        for key in ("hp", "sp", "atk", "def", "mag", "mdef", "spd"):
            stats[key] = player["base"].get(key, 0) + player["growth"].get(key, 0) * (level - 1)
        return stats
