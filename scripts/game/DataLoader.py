# -*- coding: utf-8 -*-
"""数据加载层：data/*.json 加载、缓存与基础校验。

路径常量集中在本文件；game/ 层其余模块一律通过本模块取数据。
本模块不依赖引擎（纯标准库），可独立单测。
"""

import json
import os

DATA_DIR = "data"
DIALOGUE_DIR = os.path.join(DATA_DIR, "dialogue")
BATTLE_DIR = os.path.join(DATA_DIR, "battle")

# 全量数据文件清单（阶段 2 契约）
DATA_FILES = [
    "data/dialogue/prologue.json",
    "data/dialogue/ch1.json",
    "data/battle/enemies.json",
    "data/battle/players.json",
    "data/battle/formations.json",
    "data/battle/items.json",
    "data/characters.json",
    "data/flags.json",
]

_CACHE = {}


def load_json(path):
    """读取并解析 JSON；文件缺失或解析失败直接抛异常（不做 try/except）。"""
    with open(path, "r", encoding="utf-8") as f:
        return json.load(f)


def _get(key, path, fallback=None):
    if key in _CACHE:
        return _CACHE[key]
    if not os.path.isfile(path):
        if fallback is not None:
            _CACHE[key] = fallback
            return fallback
        raise FileNotFoundError("缺少数据文件: {}".format(path))
    data = load_json(path)
    _CACHE[key] = data
    return data


def clear_cache():
    _CACHE.clear()


# ---------- 各表访问入口 ----------

def get_dialogue(file_name):
    """按文件名取对话文件（不含 .json 后缀）。"""
    return _get("dialogue:" + file_name,
                os.path.join(DIALOGUE_DIR, file_name + ".json"),
                fallback=None)


def get_enemies():
    return _get("enemies", os.path.join(BATTLE_DIR, "enemies.json"), fallback={})


def get_players():
    return _get("players", os.path.join(BATTLE_DIR, "players.json"), fallback={})


def get_formations():
    return _get("formations", os.path.join(BATTLE_DIR, "formations.json"), fallback={})


def get_items():
    return _get("items", os.path.join(BATTLE_DIR, "items.json"), fallback={})


def get_characters():
    return _get("characters", "data/characters.json", fallback={})


def get_flags_config():
    return _get("flags", "data/flags.json", fallback={})


def get_dialogue_files():
    """已固化的对话文件清单（存在才算数）。"""
    files = []
    if os.path.isfile(os.path.join(DIALOGUE_DIR, "prologue.json")):
        files.append("prologue")
    if os.path.isfile(os.path.join(DIALOGUE_DIR, "ch1.json")):
        files.append("ch1")
    return files


def find_node(file_name, node_id):
    """跨文件查询节点（当前文件优先，其次全局扫描）。"""
    if node_id is None:
        return None
    data = get_dialogue(file_name)
    if data is not None:
        for node in data.get("nodes", []):
            if node.get("id") == node_id:
                return node
    for f in get_dialogue_files():
        data = get_dialogue(f)
        if data is None:
            continue
        for node in data.get("nodes", []):
            if node.get("id") == node_id:
                return node
    return None


# ---------- 基础校验（verify_stage3 使用） ----------

def validate_dialogue(file_name, known_external=None):
    """校验单个对话文件，返回错误消息列表（空 = 通过）。"""
    errors = []
    known_external = known_external or set()
    data = get_dialogue(file_name)
    if data is None:
        return ["对话文件缺失: " + file_name]
    nodes = data.get("nodes")
    if not isinstance(nodes, list) or not nodes:
        return ["nodes 缺失或为空: " + file_name]

    ids = {}
    for node in nodes:
        nid = node.get("id")
        if not isinstance(nid, str) or not nid:
            errors.append("节点缺少字符串 id: {}".format(node))
            continue
        if nid in ids:
            errors.append("重复节点 id: {}".format(nid))
        ids[nid] = node
        if not isinstance(node.get("text"), str) or not node["text"]:
            errors.append("节点 {} 缺少 text".format(nid))
        if node.get("pose") is not None and node["pose"] not in ("left", "center", "right"):
            errors.append("节点 {} pose 非法: {}".format(nid, node["pose"]))
        if node.get("event") is not None and node["event"] not in (
                "battle_start", "battle_end", "chapter", "ending", "enter_scene"):
            errors.append("节点 {} event 非法: {}".format(nid, node["event"]))
        tw = node.get("typewriter")
        if tw is not None and (not isinstance(tw, (int, float)) or tw < 0):
            errors.append("节点 {} typewriter 非法: {}".format(nid, tw))
        options = node.get("options")
        if options is not None:
            if not isinstance(options, list) or not (1 <= len(options) <= 4):
                errors.append("节点 {} options 非法: {}".format(nid, options))
            else:
                for i, opt in enumerate(options):
                    if not isinstance(opt, dict) or not opt.get("text") or not opt.get("next"):
                        errors.append("节点 {} options[{}] 缺少 text/next".format(nid, i))

    for node in nodes:
        nid = node["id"]
        nxt = node.get("next")
        if nxt and nxt not in ids and nxt not in known_external:
            errors.append("节点 {} next 指向不存在: {}".format(nid, nxt))
        for i, opt in enumerate(node.get("options") or []):
            if isinstance(opt, dict) and opt.get("next") and \
                    opt["next"] not in ids and opt["next"] not in known_external:
                errors.append("节点 {} options[{}] next 指向不存在: {}".format(nid, i, opt["next"]))
    return errors


def validate_all(known_external=None):
    """全量数据基础校验，返回 (错误列表, 警告列表)。"""
    errors = []
    warns = []
    for path in DATA_FILES:
        if not os.path.isfile(path):
            if path.endswith("ch1.json"):
                warns.append("缺少数据文件 {}：第一章剧本待确认后固化（D-01 流程）".format(path))
            else:
                errors.append("缺少数据文件: " + path)
    if errors:
        return errors, warns

    # 对话文件
    for f in get_dialogue_files():
        errors += validate_dialogue(f, known_external)

    # enemies / players / formations / items / characters / flags 顶层结构
    enemies = get_enemies()
    for eid, enemy in enemies.items():
        for key in ("id", "name", "portrait", "hp", "atk", "def", "mag", "mdef", "spd",
                    "element", "ai", "skills"):
            if key not in enemy:
                errors.append("敌人 {} 缺少字段 {}".format(eid, key))

    players = get_players()
    for pid, p in players.items():
        for key in ("id", "name", "element", "level", "max_level", "base", "growth",
                    "skills", "portrait", "avatar"):
            if key not in p:
                errors.append("角色 {} 缺少字段 {}".format(pid, key))

    formations = get_formations()
    enemy_ids = set(enemies.keys())
    dialogue_ids = set()
    for f in get_dialogue_files():
        data = get_dialogue(f)
        if data:
            dialogue_ids.update(n["id"] for n in data.get("nodes", []))
    for fid, f in formations.items():
        for key in ("id", "name", "enemies", "boss", "allow_flee", "recommend_level",
                    "interludes", "victory", "exp_reward"):
            if key not in f:
                errors.append("编成 {} 缺少字段 {}".format(fid, key))
        for e in f.get("enemies", []):
            if isinstance(e, dict) and e.get("enemy_id") not in enemy_ids:
                if e.get("enemy_id") == "rust_commander":
                    warns.append("编成 {} 引用 {}：素材存在但 §6.5 无数值行，待阶段 4".format(fid, e["enemy_id"]))
                else:
                    errors.append("编成 {} 引用不存在敌人: {}".format(fid, e.get("enemy_id")))
        victory = f.get("victory", "")
        if victory and victory not in dialogue_ids:
            if victory.startswith("btl_b"):
                warns.append("编成 {} victory {} 属于未固化章节 dialogue".format(fid, victory))
            else:
                errors.append("编成 {} victory 指向不存在节点: {}".format(fid, victory))
        for il in f.get("interludes", []):
            if not isinstance(il, dict) or "round" not in il or "text" not in il:
                errors.append("编成 {} interludes 项缺少 round/text".format(fid))

    items = get_items()
    for iid, item in items.items():
        for key in ("id", "name", "desc", "icon", "effect_type", "effect_value", "usable_in_battle"):
            if key not in item:
                errors.append("道具 {} 缺少字段 {}".format(iid, key))

    characters = get_characters()
    for pname, pose in characters.get("poses", {}).items():
        if not isinstance(pose, dict) or "panel" not in pose or "sprite" not in pose:
            errors.append("poses.{} 缺少 panel/sprite".format(pname))

    return errors, warns
