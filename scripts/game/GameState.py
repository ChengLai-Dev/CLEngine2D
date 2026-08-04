# -*- coding: utf-8 -*-
"""全局游戏状态单例（方案文档 §3.3）。

职责：
- 我方队伍状态（出战 3 人：尹川+苏言固定，烬/栖霞按剧情解锁）
- 道具数量（经 flags 表读写）
- Flag 表（好感/visited_*，读写经 Flags.py）
- 剧情上下文（当前 dialogue 文件 / 节点 id，战斗中断与 GameOver 重试恢复用）
- 战斗返回指针（battle_start 时保存，胜利后回 DialogScene 继续）
- 战前队伍快照（GameOver 重试恢复）
- 战斗结果缓存（ResultScene 读取）
- 设置（文字速度档位，阶段 5 接 UI，先默认中速 30）
- 章节进度（供结局差分与章节过场使用）
"""

import copy

from game import DataLoader
from game import Flags

# 我方固定出战位（烬/栖霞按剧情解锁后替换 2 号位）
FIXED_PARTY = ("yinchuan", "suyan")


class GameState:
    _instance = None

    def __new__(cls):
        if cls._instance is None:
            cls._instance = super(GameState, cls).__new__(cls)
            cls._instance._initialize()
        return cls._instance

    def _initialize(self):
        self.party = {}          # pid -> {"level": int, "exp": int, "hp": int, "sp": int}
        self.flags = {}          # flag 名 -> 数值（含道具数量）
        self.dialogue_file = None
        self.dialogue_node = None
        self.battle_return_node = None   # 战斗胜利后回 DialogScene 从该节点继续
        self.battle_source = "dialog"    # 战斗来源："dialog"（剧情强制）/ "explore"（明雷）
        self.explore_return_node = None  # 探索返回 DialogScene 的节点（enter_scene 的 next）
        self.battle_snapshot = None      # 战前队伍快照（GameOver 重试恢复）
        self.battle_result = None        # 战斗结果缓存（ResultScene 读取）
        self.settings = {"text_speed": 30}   # 中速（D-07 档位：慢/中/快 = 20/30/45）
        self.chapter = 0
        self.formation_id = None         # 当前战斗编成 id（GameOver 重试用）

    # ---------- 初始化 / 重开 ----------

    def reset_new_game(self):
        """新游戏：Flags 重置、队伍初始、上下文清空。"""
        config = DataLoader.get_flags_config()
        self.flags = copy.deepcopy(config.get("initial", {}))
        self.party = {}
        for pid in FIXED_PARTY:
            p = DataLoader.get_players().get(pid)
            if p is None:
                continue
            base = p.get("base", {})
            self.party[pid] = {
                "level": p.get("level", 1),
                "exp": 0,
                "hp": base.get("hp", 1),
                "sp": base.get("sp", 0),
            }
        self.dialogue_file = None
        self.dialogue_node = None
        self.battle_return_node = None
        self.battle_source = "dialog"
        self.explore_return_node = None
        self.battle_snapshot = None
        self.battle_result = None
        self.formation_id = None
        self.chapter = 0

    def has_party(self):
        return bool(self.party)

    # ---------- Flag 读写 ----------

    def get_flag(self, name, default=0):
        return self.flags.get(name, default)

    def set_flag(self, name, value):
        self.flags[name] = value

    def eval_flag_require(self, expr):
        return Flags.eval_require(expr, self.flags)

    def apply_flag_set(self, expr):
        Flags.apply_set(expr, self.flags)
        self.check_party_join()

    def check_party_join(self):
        """角色入队钩子：flag jin_joined/qixia_joined 置位后自动解锁入队（幂等）。"""
        if self.flags.get("jin_joined", 0) > 0 and "jin" not in self.party:
            self.add_party_member("jin")
        if self.flags.get("qixia_joined", 0) > 0 and "qixia" not in self.party:
            self.add_party_member("qixia")

    # ---------- 队伍状态 ----------

    def get_member(self, pid):
        return self.party.get(pid)

    def get_member_stats(self, pid):
        """指定角色在当前等级下的满状态属性（base + growth × (level-1)）。"""
        p = DataLoader.get_players().get(pid)
        if p is None:
            return {}
        member = self.party.get(pid)
        if member is None:
            return {}
        level = member["level"]
        stats = {}
        for key in ("hp", "sp", "atk", "def", "mag", "mdef", "spd"):
            stats[key] = p["base"].get(key, 0) + p["growth"].get(key, 0) * (level - 1)
        return stats

    def get_available_skills(self, pid):
        """已解锁技能（unlock_level <= 当前等级）。"""
        p = DataLoader.get_players().get(pid)
        if p is None:
            return []
        member = self.party.get(pid)
        if member is None:
            return []
        level = member["level"]
        result = []
        for skill in p.get("skills", []):
            if skill.get("unlock_level", 1) <= level:
                result.append(skill)
        return result

    def add_party_member(self, pid):
        """按剧情解锁新队友（烬/栖霞），以当前队伍最高等级入队（追平，避免拖后腿）。"""
        if pid in self.party:
            return
        p = DataLoader.get_players().get(pid)
        if p is None:
            return
        base = p.get("base", {})
        levels = [m.get("level", 1) for m in self.party.values()]
        level = max(levels) if levels else p.get("level", 1)
        stats = {}
        for key in ("hp", "sp", "atk", "def", "mag", "mdef", "spd"):
            stats[key] = base.get(key, 0) + p.get("growth", {}).get(key, 0) * (level - 1)
        self.party[pid] = {
            "level": level,
            "exp": 0,
            "hp": stats.get("hp", 1),
            "sp": stats.get("sp", 0),
        }

    # ---------- 道具（经 flags 表） ----------

    def get_item_count(self, item_id):
        return self.flags.get("item_" + item_id, 0)

    def add_item(self, item_id, count):
        key = "item_" + item_id
        self.flags[key] = self.flags.get(key, 0) + count

    def consume_item(self, item_id):
        key = "item_" + item_id
        if self.flags.get(key, 0) <= 0:
            return False
        self.flags[key] -= 1
        return True

    # ---------- 剧情上下文 ----------

    def set_dialogue_context(self, file_name, node_id):
        self.dialogue_file = file_name
        self.dialogue_node = node_id

    def set_battle_return(self, node_id):
        self.battle_return_node = node_id

    # ---------- 战斗快照 / 结果 ----------

    def snapshot_party(self):
        """战前快照（队伍 HP/SP/等级/经验 + 道具数量），GameOver 重试恢复用。"""
        items = {}
        for key in ("item_herb", "item_dew", "item_fragment"):
            items[key] = self.flags.get(key, 0)
        self.battle_snapshot = {
            "party": copy.deepcopy(self.party),
            "items": items,
        }

    def restore_party_snapshot(self):
        """恢复战前快照：队伍状态与道具数量一并回滚。"""
        if self.battle_snapshot is None:
            return
        self.party = copy.deepcopy(self.battle_snapshot.get("party", {}))
        for key, value in self.battle_snapshot.get("items", {}).items():
            self.flags[key] = value

    def set_battle_result(self, result):
        self.battle_result = result
