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

# 全量数据文件清单（阶段 2 契约 + 阶段 4 增补 + 阶段 5 增补）
DATA_FILES = [
    "data/dialogue/prologue.json",
    "data/dialogue/ch1.json",
    "data/dialogue/ch2.json",
    "data/dialogue/ch3.json",
    "data/dialogue/final.json",
    "data/dialogue/ending.json",
    "data/battle/enemies.json",
    "data/battle/players.json",
    "data/battle/formations.json",
    "data/battle/items.json",
    "data/characters.json",
    "data/flags.json",
    "data/scenes.json",
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


def get_scenes():
    """探索场景配置表（data/scenes.json）：bg/title/hotspots/enemy/return_node。"""
    return _get("scenes", "data/scenes.json", fallback={})


def get_dialogue_files():
    """已固化的对话文件清单（存在才算数）。"""
    files = []
    if os.path.isfile(os.path.join(DIALOGUE_DIR, "prologue.json")):
        files.append("prologue")
    if os.path.isfile(os.path.join(DIALOGUE_DIR, "ch1.json")):
        files.append("ch1")
    if os.path.isfile(os.path.join(DIALOGUE_DIR, "ch2.json")):
        files.append("ch2")
    if os.path.isfile(os.path.join(DIALOGUE_DIR, "ch3.json")):
        files.append("ch3")
    if os.path.isfile(os.path.join(DIALOGUE_DIR, "final.json")):
        files.append("final")
    return files


def get_endings():
    """结局差分表（data/dialogue/ending.json）：endings.{watch|together}。"""
    return _get("endings", os.path.join(DIALOGUE_DIR, "ending.json"), fallback={})


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
    """校验单个对话文件，返回错误消息列表（空 = 通过）。
    known_external：调用方声明的合法外部引用；其余已固化对话文件的节点视为已知。"""
    errors = []
    known_external = known_external or set()
    known = set(known_external)
    for f in get_dialogue_files():
        if f == file_name:
            continue
        data = get_dialogue(f)
        if data:
            known.update(n["id"] for n in data.get("nodes", []))
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
        if nxt and nxt not in ids and nxt not in known:
            errors.append("节点 {} next 指向不存在: {}".format(nid, nxt))
        for i, opt in enumerate(node.get("options") or []):
            if isinstance(opt, dict) and opt.get("next") and \
                    opt["next"] not in ids and opt["next"] not in known:
                errors.append("节点 {} options[{}] next 指向不存在: {}".format(nid, i, opt["next"]))
    return errors


def validate_all(known_external=None):
    """全量数据基础校验，返回 (错误列表, 警告列表)。"""
    errors = []
    warns = []
    for path in DATA_FILES:
        if not os.path.isfile(path):
            errors.append("缺少数据文件: " + path)
    if errors:
        return errors, warns

    # 对话文件
    for f in get_dialogue_files():
        errors += validate_dialogue(f, known_external)

    # 后续章节（尚未固化）的跨文件节点引用降级为 WARN，不阻断
    pending_prefixes = ("ch4_",)
    kept_errors = []
    for e in errors:
        if " 指向不存在: " in e:
            target = e.rsplit(": ", 1)[-1]
            if target.startswith(pending_prefixes):
                warns.append("后续章节未固化引用（阶段 5 补齐）: " + e)
                continue
        kept_errors.append(e)
    errors = kept_errors

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

    # 结局差分表（阶段 5 新增）
    endings = get_endings()
    endings_map = endings.get("endings", {})
    if not endings_map:
        errors.append("ending.json 缺少 endings 段")
    for eid, ending in endings_map.items():
        if not isinstance(ending, dict):
            errors.append("结局 {} 不是对象".format(eid))
            continue
        for key in ("title", "extra_require", "extra_lines", "staff"):
            if key not in ending:
                errors.append("结局 {} 缺少字段 {}".format(eid, key))
        if not ending.get("title") or not ending.get("staff"):
            errors.append("结局 {} 缺少 title/staff 内容".format(eid))
        for line in ending.get("extra_lines", []):
            if not isinstance(line, dict) or not line.get("text"):
                errors.append("结局 {} extra_lines 项缺少 text".format(eid))

    items = get_items()
    for iid, item in items.items():
        for key in ("id", "name", "desc", "icon", "effect_type", "effect_value", "usable_in_battle"):
            if key not in item:
                errors.append("道具 {} 缺少字段 {}".format(iid, key))

    characters = get_characters()
    for pname, pose in characters.get("poses", {}).items():
        if not isinstance(pose, dict) or "panel" not in pose or "sprite" not in pose:
            errors.append("poses.{} 缺少 panel/sprite".format(pname))

    # 探索场景配置表（阶段 4 新增）
    scenes = get_scenes()
    formation_ids = set(formations.keys())
    for sid, scene in scenes.items():
        if not isinstance(scene, dict):
            errors.append("场景 {} 不是对象".format(sid))
            continue
        if not scene.get("bg") or not scene.get("title"):
            errors.append("场景 {} 缺少 bg/title".format(sid))
        hotspots = scene.get("hotspots")
        if not isinstance(hotspots, list) or not (1 <= len(hotspots) <= 3):
            errors.append("场景 {} hotspots 数量非法（1~3）".format(sid))
        else:
            for i, h in enumerate(hotspots):
                if not isinstance(h, dict) or not h.get("text"):
                    errors.append("场景 {} hotspots[{}] 缺少 text".format(sid, i))
                elif h.get("action") == "battle" and h.get("formation") not in formation_ids:
                    errors.append("场景 {} hotspots[{}] formation 不存在: {}".format(
                        sid, i, h.get("formation")))
                elif h.get("action") == "dialogue" and not h.get("target"):
                    errors.append("场景 {} hotspots[{}] 缺少 target".format(sid, i))
        enemy = scene.get("enemy")
        if enemy is not None:
            if not enemy.get("texture") or not enemy.get("name"):
                errors.append("场景 {} enemy 缺少 texture/name".format(sid))
            if enemy.get("formation") not in formation_ids:
                errors.append("场景 {} enemy.formation 不存在: {}".format(
                    sid, enemy.get("formation")))
        if not scene.get("return_node"):
            errors.append("场景 {} 缺少 return_node".format(sid))

    return errors, warns
