"""阶段 2 验证脚本：data/ 数据层全量校验（方案文档 §5 数据契约）。

运行方式（工作目录 = 项目根）：
    out/build/default/src/py_sandbox/Debug/py_sandbox.exe --module verify_stage2

验证点：
  1. data/ 全部 JSON 解析通过（失败即红：json.load 直接抛错，退出码非 0）
  2. 各表必填字段/类型校验（方案文档 §5 字段清单）
  3. dialogue：id 唯一、next/options.next/event_param 引用完整、ID 前缀规范（pro_）
  4. enemies.json 与策划案 §6.5 抽 3 行逐字段核对
  5. 交叉引用：formations->enemies/dialogue、players->characters、贴图路径文件存在
  6. 序章字数统计（2,500 字 ±10%）与伤害公式手算样例打印
"""

import json
import os
import re

DIALOGUE_FILES = ["data/dialogue/prologue.json"]
DATA_FILES = [
    "data/dialogue/prologue.json",
    "data/battle/enemies.json",
    "data/battle/players.json",
    "data/battle/formations.json",
    "data/battle/items.json",
    "data/characters.json",
    "data/flags.json",
]

EVENTS = ("battle_start", "battle_end", "chapter", "ending", "enter_scene")
ENEMY_EFFECTS = ("伤害", "降攻", "封印", "防御up", "蓄力", "禁SP")
PLAYER_EFFECTS = ("伤害", "回复", "降防", "防御up", "速度up", "减速", "护盾")
POSES = ("left", "center", "right")

FLAG_SET_RE = re.compile(r"^[a-z0-9_]+=[+-]?\d+(;[a-z0-9_]+=[+-]?\d+)*$")
FLAG_REQ_RE = re.compile(r"^[a-z0-9_]+(>=|<=|>|<|=)\d+(&&[a-z0-9_]+(>=|<=|>|<|=)\d+)*$")

# 策划案 §6.5 抽查期望值（抽 3 行：蚀页魔 / 锈铁傀儡 / 无面者·真）
ENEMY_EXPECT = {
    "erosion_moth": {"hp": 45, "atk": 10, "def": 5, "mag": 8, "mdef": 5, "spd": 6, "element": "暗"},
    "rust_golem": {"hp": 90, "atk": 15, "def": 12, "mag": 0, "mdef": 8, "spd": 4, "element": "无"},
    "faceless_true": {"hp": 260, "atk": 26, "def": 12, "mag": 30, "mdef": 18, "spd": 16, "element": "暗"},
}

# 策划案 §6.4 成长公式（逐字段核对）
GROWTH_EXPECT = {"hp": 12, "sp": 4, "atk": 2, "def": 1, "mag": 2, "mdef": 1, "spd": 1}

failed = 0
WARNED = 0
EXTERNAL_NODES = set()
PORTRAIT_IDS = set()
# 素材存在但策划案 §6.5 无数值行的精英（阶段 4 前待开发者补数值）
KNOWN_PENDING_ENEMIES = {"rust_commander"}


def log(msg):
    print(msg, flush=True)


def warn(tag, detail):
    global WARNED
    print(f"[verify] WARN: {tag} - {detail}", flush=True)
    WARNED += 1


def check(tag, ok, detail):
    global failed
    print(f"[verify] {tag}: {'OK' if ok else 'FAIL'} - {detail}", flush=True)
    if not ok:
        failed += 1


def load_json(path):
    with open(path, "r", encoding="utf-8") as f:
        return json.load(f)


def require_fields(obj, fields, path):
    missing = [f for f in fields if f not in obj]
    return missing


def check_optional_str(obj, key, path):
    if key in obj and not isinstance(obj[key], str):
        check(f"{path}.{key} 类型", False, f"应为 str，实际 {type(obj[key]).__name__}")
        return False
    return True


def check_text_nodes(nodes):
    """校验单个对话文件：id 唯一/必填字段/引用完整性。"""
    ids = {}
    for node in nodes:
        nid = node.get("id", "")
        if not isinstance(nid, str) or not nid:
            check("节点 id", False, f"节点缺少字符串 id: {node}")
            continue
        if nid in ids:
            check(f"节点 id 唯一", False, f"重复 id: {nid}")
        ids[nid] = node

        tag = f"节点 {nid}"
        missing = require_fields(node, ["id", "text"], tag)
        if missing:
            check(f"{tag} 必填字段", False, f"缺失: {missing}")

        if not isinstance(node.get("text"), str) or not node["text"]:
            check(f"{tag}.text", False, "正文缺失或非字符串")

        if node.get("pose") is not None and node["pose"] not in POSES:
            check(f"{tag}.pose", False, f"非法站位: {node['pose']}")

        if node.get("event") is not None and node["event"] not in EVENTS:
            check(f"{tag}.event", False, f"非法事件: {node['event']}")

        tw = node.get("typewriter")
        if tw is not None and (not isinstance(tw, (int, float)) or tw < 0):
            check(f"{tag}.typewriter", False, f"非法值: {tw}")

        if node.get("flag_set") is not None and not FLAG_SET_RE.match(node["flag_set"]):
            check(f"{tag}.flag_set", False, f"非法赋值表达式: {node['flag_set']}")

        if node.get("flag_require") is not None and not FLAG_REQ_RE.match(node["flag_require"]):
            check(f"{tag}.flag_require", False, f"非法条件表达式: {node['flag_require']}")

        for key in ("speaker", "portrait", "next", "event_param", "bgm", "sound"):
            check_optional_str(node, key, tag)

        options = node.get("options")
        if options is not None:
            if not isinstance(options, list) or len(options) > 4:
                check(f"{tag}.options", False, "options 必须为数组且最多 4 项")
                continue
            for i, opt in enumerate(options):
                if not isinstance(opt, dict) or "text" not in opt or "next" not in opt:
                    check(f"{tag}.options[{i}]", False, "选项缺少 text/next")
                if "flag_set" in opt and not FLAG_SET_RE.match(opt["flag_set"]):
                    check(f"{tag}.options[{i}].flag_set", False, f"非法表达式: {opt['flag_set']}")

    for node in nodes:
        nid = node["id"]
        nxt = node.get("next")
        if nxt and nxt not in ids and nxt not in EXTERNAL_NODES:
            check(f"节点 {nid}.next 引用", False, f"指向不存在的节点: {nxt}")
        for i, opt in enumerate(node.get("options") or []):
            if isinstance(opt, dict) and opt.get("next") and opt["next"] not in ids and opt["next"] not in EXTERNAL_NODES:
                check(f"节点 {nid}.options[{i}].next 引用", False, f"指向不存在的节点: {opt['next']}")


EXTERNAL_NODES = set()


def verify_dialogue():
    all_ids = set()
    for path in DIALOGUE_FILES:
        if not os.path.isfile(path):
            check(f"对话文件存在 {path}", False, "文件缺失（序章剧本未固化？）")
            continue
        data = load_json(path)
        if data.get("file") != os.path.splitext(os.path.basename(path))[0]:
            check(f"{path}.file 字段", False, f"应为 {os.path.basename(path)[:-5]}")
        nodes = data.get("nodes", [])
        if not isinstance(nodes, list) or not nodes:
            check(f"{path}.nodes", False, "nodes 缺失或为空")
            continue
        check_text_nodes(nodes)
        ids = [n["id"] for n in nodes]
        all_ids.update(ids)
        dup = len(ids) != len(set(ids))
        check(f"{path} 节点 id 无重复", not dup, f"{len(ids)} 个节点")

    for path in DIALOGUE_FILES:
        if not os.path.isfile(path):
            continue
        nodes = load_json(path)["nodes"]
        for node in nodes:
            if node["id"].startswith("pro_") or node["id"] == "btl_b1_result":
                pass
            else:
                check("节点 ID 前缀规范", False, f"非法前缀: {node['id']}")

    total_chars = 0
    for path in DIALOGUE_FILES:
        if not os.path.isfile(path):
            continue
        nodes = load_json(path)["nodes"]
        for node in nodes:
            total_chars += len(node["text"])
    check("序章字数 2500±10%", 2250 <= total_chars <= 2750, f"{total_chars} 字")

    if "btl_b1_result" in all_ids:
        check("battle_end 节点存在", True, "btl_b1_result 已定义")


def verify_enemies():
    data = load_json("data/battle/enemies.json")
    check("enemies.json 顶层 dict", isinstance(data, dict), f"{len(data)} 种敌人")

    for eid, enemy in data.items():
        tag = f"敌人 {eid}"
        missing = require_fields(enemy, ["id", "name", "portrait", "hp", "atk", "def", "mag", "mdef", "spd", "element", "ai", "skills"], tag)
        if missing:
            check(f"{tag} 必填字段", False, f"缺失: {missing}")
        for key in ("hp", "atk", "def", "mag", "mdef", "spd"):
            if not isinstance(enemy.get(key), (int, float)):
                check(f"{tag}.{key}", False, "非数值")
        for skill in enemy.get("skills", []):
            sm = require_fields(skill, ["id", "name", "type", "target", "multiplier", "effect", "effect_value", "duration", "chance"], f"{tag}.skills")
            if sm:
                check(f"{tag}.skills 必填字段", False, f"缺失: {sm}")
            if skill.get("type") not in ("物理", "魔法"):
                check(f"{tag}.skills.type", False, f"非法: {skill.get('type')}")
            if skill.get("target") not in ("单体", "全体"):
                check(f"{tag}.skills.target", False, f"非法: {skill.get('target')}")
            if skill.get("effect") not in ENEMY_EFFECTS:
                check(f"{tag}.skills.effect", False, f"非法效果: {skill.get('effect')}")
            if not isinstance(skill.get("chance"), (int, float)) or not (0 <= skill["chance"] <= 1):
                check(f"{tag}.skills.chance", False, f"概率越界: {skill.get('chance')}")

    for eid, expect in ENEMY_EXPECT.items():
        enemy = data.get(eid)
        if enemy is None:
            check(f"§6.5 抽查 {eid} 存在", False, "敌人缺失")
            continue
        bad = [k for k, v in expect.items() if enemy.get(k) != v]
        check(f"§6.5 抽查 {eid} 数值一致", not bad, f"不一致字段: {bad if bad else '无'}")

    expect_skills = {
        "erosion_moth": [("撕咬", 1.0, "单体")],
        "rust_golem": [("铁壁", 0.0, "单体")],
        "faceless_true": [("遗忘漩涡", 1.0, "全体"), ("终焉之书", 2.0, "全体")],
    }
    for eid, skill_list in expect_skills.items():
        enemy = data.get(eid)
        if enemy is None:
            continue
        got = [(s["name"], s["multiplier"], s["target"]) for s in enemy["skills"]]
        check(f"§6.5 抽查 {eid} 技能一致", got == skill_list, f"{got}")


def verify_players():
    data = load_json("data/battle/players.json")
    check("players.json 顶层 dict", isinstance(data, dict), f"{len(data)} 名角色")

    for pid, p in data.items():
        tag = f"角色 {pid}"
        missing = require_fields(p, ["id", "name", "element", "level", "max_level", "base", "growth", "skills", "portrait", "avatar"], tag)
        if missing:
            check(f"{tag} 必填字段", False, f"缺失: {missing}")
        if p.get("level") != 1:
            check(f"{tag}.level", False, f"初始应为 1，实际 {p.get('level')}")
        if p.get("max_level") != 12:
            check(f"{tag}.max_level", False, f"应为 12，实际 {p.get('max_level')}")
        for key in ("hp", "sp", "atk", "def", "mag", "mdef", "spd"):
            if not isinstance(p.get("base", {}).get(key), (int, float)):
                check(f"{tag}.base.{key}", False, "非数值")
        growth = p.get("growth", {})
        bad_g = [k for k, v in GROWTH_EXPECT.items() if growth.get(k) != v]
        check(f"{tag}.growth 与 §6.4 一致", not bad_g, f"不一致: {bad_g if bad_g else '无'}")
        skills = p.get("skills", [])
        if len(skills) != 3:
            check(f"{tag}.skills 数量", False, f"应为 3，实际 {len(skills)}")
        for skill in skills:
            sm = require_fields(skill, ["id", "name", "power", "element", "type", "effect", "desc"], f"{tag}.skills")
            if sm:
                check(f"{tag}.skills 必填字段", False, f"缺失: {sm}")
            if skill.get("type") not in ("物理", "魔法"):
                check(f"{tag}.skills.type", False, f"非法: {skill.get('type')}")
            if skill.get("effect") not in PLAYER_EFFECTS:
                check(f"{tag}.skills.effect", False, f"非法效果: {skill.get('effect')}")


def verify_formations(enemy_ids, dialogue_ids):
    data = load_json("data/battle/formations.json")
    check("formations.json 顶层 dict", isinstance(data, dict), f"{len(data)} 场编成")
    expect_ids = ["b1", "b2", "b3", "b4", "b5", "b6", "b7", "b8"]
    missing = [i for i in expect_ids if i not in data]
    check("8 场编成齐全", not missing, f"缺失: {missing if missing else '无'}")

    for fid, f in data.items():
        tag = f"编成 {fid}"
        missing = require_fields(f, ["id", "name", "enemies", "boss", "allow_flee", "recommend_level", "interludes", "victory", "exp_reward"], tag)
        if missing:
            check(f"{tag} 必填字段", False, f"缺失: {missing}")
        if not f.get("enemies"):
            warn(f"{tag}.enemies", "编成为空（骨架/待补）")
        for e in f.get("enemies", []):
            if not isinstance(e, dict) or "enemy_id" not in e or "count" not in e:
                check(f"{tag}.enemies 项", False, "缺少 enemy_id/count")
                continue
            if e["enemy_id"] not in enemy_ids:
                if e["enemy_id"] in KNOWN_PENDING_ENEMIES:
                    warn(f"{tag} 敌人引用 {e['enemy_id']}", "素材存在但 §6.5 无数值行，待阶段 4 补")
                else:
                    check(f"{tag} 敌人引用 {e['enemy_id']}", False, "enemies.json 中不存在该敌人")
        victory = f.get("victory", "")
        if victory and victory not in dialogue_ids and not victory.startswith("btl_b"):
            check(f"{tag}.victory 引用", False, f"dialogue 中无此节点: {victory}")
        elif victory and victory not in dialogue_ids:
            warn(f"{tag}.victory", f"{victory} 属于后续章节 dialogue，待对应章节固化")
        for il in f.get("interludes", []):
            im = require_fields(il, ["round", "speaker", "portrait", "text"], f"{tag}.interludes")
            if im:
                check(f"{tag}.interludes 必填字段", False, f"缺失: {im}")
            if il.get("portrait") not in PORTRAIT_IDS:
                check(f"{tag}.interludes.portrait", False, f"立绘不存在: {il.get('portrait')}")


def verify_items():
    data = load_json("data/battle/items.json")
    check("items.json 顶层 dict", isinstance(data, dict), f"{len(data)} 种道具")
    for iid, item in data.items():
        tag = f"道具 {iid}"
        missing = require_fields(item, ["id", "name", "desc", "icon", "effect_type", "effect_value", "usable_in_battle"], tag)
        if missing:
            check(f"{tag} 必填字段", False, f"缺失: {missing}")
        if not isinstance(item.get("usable_in_battle"), bool):
            check(f"{tag}.usable_in_battle", False, "应为 bool")
    herb = data.get("herb", {})
    dew = data.get("dew", {})
    frag = data.get("fragment", {})
    check("herb 回复 HP 60", herb.get("effect_type") == "heal_hp" and herb.get("effect_value") == 60, f"{herb.get('effect_type')}/{herb.get('effect_value')}")
    check("dew 回复 SP 30", dew.get("effect_type") == "heal_sp" and dew.get("effect_value") == 30, f"{dew.get('effect_type')}/{dew.get('effect_value')}")
    check("fragment 战斗不可用", frag.get("usable_in_battle") is False, f"{frag.get('usable_in_battle')}")


def verify_characters():
    global PORTRAIT_IDS
    data = load_json("data/characters.json")
    portraits = data.get("portraits", {})
    poses = data.get("poses", {})
    PORTRAIT_IDS = set(portraits.keys())
    check("characters.json portraits dict", isinstance(portraits, dict), f"{len(portraits)} 张立绘")
    check("poses 三站位齐全", set(poses.keys()) == set(POSES), f"{list(poses.keys())}")
    for pname, pose in poses.items():
        if not isinstance(pose, dict) or "panel" not in pose or "sprite" not in pose:
            check(f"poses.{pname}", False, "缺少 panel/sprite")


def verify_flags():
    data = load_json("data/flags.json")
    initial = data.get("initial", {})
    ending = data.get("ending", {})
    for f in ("aff_suyan", "aff_jin", "aff_qixia"):
        if initial.get(f) != 0:
            check(f"flags.initial.{f}", False, f"应为 0，实际 {initial.get(f)}")
    check("ending.threshold", ending.get("threshold") == 5, f"{ending.get('threshold')}")
    if "watch_extra" in ending and not FLAG_REQ_RE.match(ending["watch_extra"]):
        check("ending.watch_extra 表达式", False, ending["watch_extra"])


def verify_asset_files():
    refs = []
    portraits = load_json("data/characters.json")["portraits"]
    refs += [(p, "texture") for p in portraits.values()]
    refs += [(p, "avatar") for p in portraits.values()]
    for enemy in load_json("data/battle/enemies.json").values():
        refs.append((enemy, "portrait"))
    for item in load_json("data/battle/items.json").values():
        refs.append((item, "icon"))
    seen = set()
    for obj, key in refs:
        path = obj.get(key)
        if path in seen:
            continue
        seen.add(path)
        if not os.path.isfile(path):
            check(f"贴图文件存在 {path}", False, "文件缺失")


def print_damage_samples():
    log("")
    log("=== 伤害公式手算样例（方案 §6.4，供阶段 3/4 对照）===")
    log("物理 = (攻×倍率 − 防×0.6) × 克制 × 浮动(0.9~1.1)；魔法 = (魔×倍率 − 魔防×0.5) × 克制 × 浮动")
    log("样例1 尹川1级普攻(攻20,光) vs 蚀页魔(防5,暗,光克暗1.5):")
    log("  (20×1.0−5×0.6)×1.5=25.5 → ×0.9~1.1 = 22.95~28.05 ≈ 23~28（与策划案 §6.5 核查一致）")
    log("样例2 尹川1级普攻 vs 锈铁傀儡(防12,无,系数1.0):")
    log("  (20×1.0−12×0.6)×1.0=12.8 → ×0.9~1.1 = 11.52~14.08 ≈ 12~14")
    log("样例3 苏言1级水刃(魔20,倍率1.2) vs 蚀页魔(魔防5,水vs暗 系数1.0):")
    log("  (20×1.2−5×0.5)×1.0=21.5 → ×0.9~1.1 = 19.35~23.65 ≈ 19~24")
    log("样例4 样例1暴击(5%,×1.5): 25.5×1.5=38.25 → ×0.9~1.1 = 34.43~42.08 ≈ 34~42")
    log("保底 1 点；暴击率 5% 仅攻击技能，伤害数字以不同颜色/字号展示（D3/D4）")


def on_init():
    log("[verify] 阶段 2 数据层校验开始")

    missing_files = [p for p in DATA_FILES if not os.path.isfile(p)]
    check("data/ 全部文件存在", not missing_files, f"缺失: {missing_files if missing_files else '无'}")

    EXTERNAL_NODES.clear()
    EXTERNAL_NODES.update(["ch1_001", "battle_b1"])

    verify_dialogue()

    enemy_ids = set()
    if os.path.isfile("data/battle/enemies.json"):
        verify_enemies()
        enemy_ids = set(load_json("data/battle/enemies.json").keys())

    if os.path.isfile("data/battle/players.json"):
        verify_players()

    if os.path.isfile("data/characters.json"):
        verify_characters()

    dialogue_ids = set()
    for path in DIALOGUE_FILES:
        if os.path.isfile(path):
            dialogue_ids.update(n["id"] for n in load_json(path)["nodes"])

    if os.path.isfile("data/battle/formations.json"):
        verify_formations(enemy_ids, dialogue_ids)

    if os.path.isfile("data/battle/items.json"):
        verify_items()

    if os.path.isfile("data/flags.json"):
        verify_flags()

    if all(os.path.isfile(p) for p in ["data/characters.json", "data/battle/enemies.json", "data/battle/items.json"]):
        verify_asset_files()

    log("")
    log(f"[verify] 校验完成：失败 {failed} 项")
    if failed == 0:
        log("[verify] 阶段 2 数据层全部通过")
    else:
        log("[verify] 存在失败项，请检查上述 FAIL 输出")
    print_damage_samples()


def on_update(dt):
    pass


def on_render():
    pass


def on_shutdown():
    pass
