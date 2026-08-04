# -*- coding: utf-8 -*-
"""阶段 3 验证脚本：叙事管线（game/ 层纯逻辑，不依赖引擎 UI）。

运行方式（两种均可，工作目录 = 项目根）：
    python scripts/verify_stage3.py
    out/build/default/src/py_sandbox/Debug/py_sandbox.exe --module verify_stage3

验证点：
  1. DataLoader 全量加载与基础校验（8 个 JSON）
  2. 序章链路走查：pro_001 → … → pro_030（选项两路 + Flag 差分 aff_suyan=+1/+2）
     → pro_035（battle_start/b1）→ btl_b1_result（battle_end）→ pro_041（chapter）→ ch1_001
  3. 10 条示例节点解析断言（序章.md 对照表）
  4. BattleEngine 伤害公式抽 3 组对照阶段 2 交付说明 §5 手算样例（采样区间断言）
  5. B1 编成插话回合（round 1/2/3）与 exp_reward=30、升级曲线（每级 100）
"""

import sys
import os

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from game import DataLoader
from game.GameState import GameState
from game.DialogueEngine import DialogueEngine
from game.BattleEngine import BattleEngine

failed = 0
WARNED = 0


def log(msg):
    print(msg, flush=True)


def warn(tag, detail):
    global WARNED
    print("[verify] WARN: {} - {}".format(tag, detail), flush=True)
    WARNED += 1


def check(tag, ok, detail):
    global failed
    print("[verify] {}: {} - {}".format(tag, "OK" if ok else "FAIL", detail), flush=True)
    if not ok:
        failed += 1


# ---------- 1. DataLoader 全量 ----------

def verify_data_loader():
    log("")
    log("=== 1. DataLoader 全量加载与基础校验 ===")
    errors, warns = DataLoader.validate_all(known_external={"battle_b1", "ch1_001"})
    check("全量数据文件加载", not errors, "{} 个错误".format(len(errors)))
    for e in errors:
        log("    ERR: " + e)
    for w in warns:
        warn("数据", w)
    formations = DataLoader.get_formations()
    check("8 场编成齐全", len(formations) == 8, "{} 场".format(len(formations)))
    b1 = formations.get("b1", {})
    check("b1.exp_reward=30（开发者已确认）", b1.get("exp_reward") == 30,
          "exp_reward={}".format(b1.get("exp_reward")))
    check("b1.victory=btl_b1_result", b1.get("victory") == "btl_b1_result",
          b1.get("victory"))


# ---------- 2/3. 序章链路走查 + 示例节点断言 ----------

def _walk_to(engine, target_id, max_steps=200):
    """从当前节点连续 advance 到目标节点；返回是否到达。"""
    steps = 0
    while engine.node_id != target_id and steps < max_steps:
        prev = engine.node_id
        engine.advance()
        if engine.node_id == prev:
            return False
        steps += 1
    return engine.node_id == target_id


def _expect_node(engine, node_id, expect):
    """断言当前节点字段（expect 为字段名 -> 期望值）。"""
    node = engine.get_node()
    for key, value in expect.items():
        got = node.get(key) if node else None
        check("节点 {} .{}".format(node_id, key), got == value, "{} -> {}".format(got, value))


def verify_example_nodes():
    """序章.md 10 条可核对条目解析断言。"""
    log("")
    log("=== 2. 10 条示例节点解析 ===")
    gs = GameState()
    gs.reset_new_game()
    eng = DialogueEngine(gs)

    # 1 标准对话（示例 1）
    node = DataLoader.find_node("prologue", "pro_017")
    check("示例1 pro_017 speaker/portrait/typewriter/bgm",
          node.get("speaker") == "苏言" and node.get("portrait") == "suyan_normal"
          and node.get("typewriter") == 25 and node.get("bgm") == "theme_suyan",
          str({k: node.get(k) for k in ("speaker", "portrait", "typewriter", "bgm")}))
    # 2 旁白（示例 2）
    node = DataLoader.find_node("prologue", "pro_001")
    check("示例2 pro_001 无 speaker/无立绘/typewriter=20",
          node.get("speaker") is None and node.get("portrait") is None
          and node.get("typewriter") == 20, str(node.get("typewriter")))
    # 3 换表情换位（示例 3）
    node = DataLoader.find_node("prologue", "pro_010")
    check("示例3 pro_010 表情/站位", node.get("portrait") == "shenyanqiu_serious"
          and node.get("pose") == "left", "{}/{}".format(node.get("portrait"), node.get("pose")))
    # 4 选项节点（示例 4）
    node = DataLoader.find_node("prologue", "pro_030")
    opts = node.get("options") or []
    check("示例4 pro_030 两选项各带 next/flag_set",
          len(opts) == 2 and all("next" in o and "flag_set" in o for o in opts),
          str(opts))
    # 5 战斗前哨（示例 5）
    node = DataLoader.find_node("prologue", "pro_035")
    check("示例5 pro_035 battle_start/b1",
          node.get("event") == "battle_start" and node.get("event_param") == "b1",
          "{}/{}".format(node.get("event"), node.get("event_param")))
    # 6 Flag 差分（示例 6，两路 flag_set）
    node = DataLoader.find_node("prologue", "pro_030")
    sets = [o.get("flag_set") for o in opts]
    check("示例6 pro_030 flag_set=+1/+2", "aff_suyan=+1" in sets and "aff_suyan=+2" in sets,
          str(sets))
    # 7 战斗胜利回归（示例 8）
    node = DataLoader.find_node("prologue", "btl_b1_result")
    check("示例8 btl_b1_result battle_end/next",
          node.get("event") == "battle_end" and node.get("next") == "pro_036",
          "{}/{}".format(node.get("event"), node.get("next")))
    # 8 章节过场（示例 9）
    node = DataLoader.find_node("prologue", "pro_041")
    check("示例9 pro_041 chapter/1",
          node.get("event") == "chapter" and node.get("event_param") == "1",
          "{}/{}".format(node.get("event"), node.get("event_param")))
    # 9 纯旁白叙事（示例 2 风格）
    node = DataLoader.find_node("prologue", "pro_013")
    check("示例9 pro_013 纯旁白", node.get("speaker") is None and node.get("portrait") is None,
          "speaker={}".format(node.get("speaker")))
    # 10 短对白（示例 1 风格）
    node = DataLoader.find_node("prologue", "pro_021")
    check("示例10 pro_021 短对白", node.get("speaker") == "苏言"
          and node.get("portrait") == "suyan_normal",
          "speaker={}".format(node.get("speaker")))


def verify_dialogue_walkthrough():
    log("")
    log("=== 3. 序章链路走查（含选项两路与事件分发） ===")
    gs = GameState()
    eng = DialogueEngine(gs)

    # 路 A：跟着苏言走（aff_suyan=+1）
    gs.reset_new_game()
    eng.start("prologue", "pro_001")
    ok = _walk_to(eng, "pro_030")
    check("链路 pro_001→pro_030", ok, "node={}".format(eng.node_id))
    if ok:
        target = eng.choose_option(0)
        check("选项A → pro_031a", target == "pro_031a", str(target))
        check("选项A aff_suyan=+1", gs.get_flag("aff_suyan") == 1,
              "aff_suyan={}".format(gs.get_flag("aff_suyan")))
        check("选项A 差分文本", eng.get_node().get("text", "").startswith("你倒是干脆。"),
              eng.get_node().get("text", "")[:12])
        ok2 = _walk_to(eng, "pro_035")
        check("链路 → pro_035(battle_start)", ok2, "node={}".format(eng.node_id))
        if ok2:
            ev, param = eng.get_event()
            check("事件 battle_start/b1", ev == "battle_start" and param == "b1",
                  "{}/{}".format(ev, param))
        eng.start("prologue", "btl_b1_result")
        ev, param = eng.get_event()
        check("事件 battle_end（victory 返回节点）", ev == "battle_end", str(ev))
        eng.advance()
        check("battle_end → pro_036", eng.node_id == "pro_036", eng.node_id)
        ok3 = _walk_to(eng, "pro_041")
        check("链路 → pro_041(chapter)", ok3, "node={}".format(eng.node_id))
        if ok3:
            ev, param = eng.get_event()
            check("事件 chapter/1", ev == "chapter" and param == "1", "{}/{}".format(ev, param))
            eng.advance()
            check("chapter → ch1_001", eng.node_id == "ch1_001", eng.node_id)
            if os.path.isfile(os.path.join("data", "dialogue", "ch1.json")):
                check("ch1_001 节点可查", eng.get_node() is not None, str(eng.node_id))
            else:
                warn("ch1.json", "未固化（D-01 流程中），ch1_001 节点暂不可查")

    # 路 B：先把馆长的座椅搬正（aff_suyan=+2）
    gs.reset_new_game()
    eng.start("prologue", "pro_001")
    _walk_to(eng, "pro_030")
    target = eng.choose_option(1)
    check("选项B → pro_031b", target == "pro_031b", str(target))
    check("选项B aff_suyan=+2", gs.get_flag("aff_suyan") == 2,
          "aff_suyan={}".format(gs.get_flag("aff_suyan")))
    check("选项B 差分文本", eng.get_node().get("text", "").startswith("苏言停在门边"),
          eng.get_node().get("text", "")[:12])


# ---------- 4. BattleEngine 伤害公式抽 3 组（对照阶段 2 §5） ----------

def _sample_damage(engine, attacker, target, skill, crit_expected=False, count=300):
    """对同一目标反复结算伤害，返回非暴击样本列表（每次结算前重置目标）。"""
    samples = []
    for _ in range(count):
        events = engine._apply_damage_skill(attacker, target, skill)
        for ev in events:
            if ev.get("type") == "damage" and ev.get("crit") is False:
                samples.append(ev.get("value"))
        target.hp = target.max_hp
        target.dead = False
    return samples


def verify_damage_formula():
    log("")
    log("=== 4. 伤害公式抽 3 组（对照阶段 2 §5 手算样例） ===")
    gs = GameState()
    gs.reset_new_game()
    eng = BattleEngine(gs)
    eng.start_battle("b1")
    yinchuan = eng.get_player_units()[0]
    suyan = eng.get_player_units()[1]
    moth = eng.get_alive_enemies()[0]

    attack_skill = {"name": "攻击", "type": "物理", "target": "单体",
                    "multiplier": 1.0, "element": "光"}

    # 样例 1：尹川 1 级普攻（攻 20，光）vs 蚀页魔（防 5，暗，光克暗 1.5）
    # 手算：(20×1.0−5×0.6)×1.5=25.5 → ×0.9~1.1 ≈ 23~28（int 舍入后 22~28）
    samples = _sample_damage(eng, yinchuan, moth, attack_skill)
    s1 = (min(samples), max(samples)) if samples else (None, None)
    check("样例1 尹川普攻 vs 蚀页魔 区间 22~28（手算 ≈23~28）",
           samples and 22 <= s1[0] and s1[1] <= 28,
           "实测 {} ~ {}（{} 次采样）".format(s1[0], s1[1], len(samples)))

    # 样例 3：苏言 1 级水刃（魔 20，倍率 1.2）vs 蚀页魔（魔防 5，水 vs 暗 1.0）
    # 手算：(20×1.2−5×0.5)×1.0=21.5 → ×0.9~1.1 ≈ 19~24（int 舍入后 19~23）
    water_skill = {"name": "水刃", "type": "魔法", "target": "单体",
                   "multiplier": 1.2, "element": "水"}
    samples = _sample_damage(eng, suyan, moth, water_skill)
    s3 = (min(samples), max(samples)) if samples else (None, None)
    check("样例3 苏言水刃 vs 蚀页魔 区间 19~23（手算 ≈19~24）",
           samples and 19 <= s3[0] and s3[1] <= 23,
           "实测 {} ~ {}（{} 次采样）".format(s3[0], s3[1], len(samples)))

    # 样例 2：尹川 1 级普攻 vs 锈铁傀儡（防 12，无属性 1.0）
    # 手算：(20×1.0−12×0.6)×1.0=12.8 → ×0.9~1.1 ≈ 12~14（int 舍入后 11~14）
    eng2 = BattleEngine(GameState())
    gs2 = GameState()
    gs2.reset_new_game()
    eng2 = BattleEngine(gs2)
    eng2.start_battle("b3")
    yc2 = eng2.get_player_units()[0]
    golem = None
    for u in eng2.units:
        if u.side == "enemy" and u.name == "锈铁傀儡":
            golem = u
            break
    if golem is not None:
        samples = _sample_damage(eng2, yc2, golem, attack_skill)
        s2 = (min(samples), max(samples)) if samples else (None, None)
        check("样例2 尹川普攻 vs 锈铁傀儡 区间 11~14（手算 ≈12~14）",
               samples and 11 <= s2[0] and s2[1] <= 14,
               "实测 {} ~ {}（{} 次采样）".format(s2[0], s2[1], len(samples)))
    else:
        check("样例2 锈铁傀儡可查（b3 编成）", False, "b3.enemies 无 rust_golem")

    # 暴击（样例 4：×1.5，金色数字由场景层展示，此处只验证倍率）
    crits = []
    for _ in range(2000):
        events = eng._apply_damage_skill(yinchuan, moth, attack_skill)
        for ev in events:
            if ev.get("type") == "damage" and ev.get("crit"):
                crits.append(ev.get("value"))
        moth.hp = moth.max_hp
        moth.dead = False
    if crits:
        check("样例4 暴击 ×1.5 生效（约 34~42）",
               all(34 <= c <= 42 for c in crits),
               "实测 {} 次暴击，区间 {} ~ {}".format(len(crits), min(crits), max(crits)))
    else:
        check("样例4 暴击采样", False, "2000 次采样无暴击（5% 概率不应如此）")


# ---------- 5. B1 插话回合 / 升级曲线 ----------

def verify_interludes_and_level():
    log("")
    log("=== 5. B1 插话回合与升级曲线 ===")
    formations = DataLoader.get_formations()
    b1 = formations.get("b1", {})
    rounds = [il.get("round") for il in b1.get("interludes", [])]
    check("B1 插话回合 1/2/3（验收④）", sorted(rounds) == [1, 2, 3], str(rounds))
    for r in (1, 2, 3):
        il = b1["interludes"][r - 1]
        check("插话 {} 说话人=苏言/有立绘".format(r),
              il.get("speaker") == "苏言" and il.get("portrait") in
              ("suyan_normal", "suyan_smile"), "{}/{}".format(il.get("speaker"), il.get("portrait")))

    gs = GameState()
    gs.reset_new_game()
    eng = BattleEngine(gs)
    eng.start_battle("b1")
    check("B1 编成敌人 2 只蚀页魔",
          len(eng.get_alive_enemies()) == 2 and all(u.name == "蚀页魔" for u in eng.get_alive_enemies()),
          str([u.name for u in eng.get_alive_enemies()]))
    check("行动顺序：速度降序、同速我方优先（尹川7/苏言6/魔6）",
          [u.name for u in eng.get_turn_order()] == ["尹川", "苏言", "蚀页魔", "蚀页魔"],
          str([u.name for u in eng.get_turn_order()]))

    # 升级：每级 100 经验；B1 给 30 → 不升级；累计 100 → Lv2 成长
    yc = gs.get_member("yinchuan")
    before = dict(yc)
    ups = eng.gain_exp(30)
    check("B1=30 经验不升级（Lv1 exp=30）",
          not ups and yc["exp"] == 30, "exp={}".format(yc["exp"]))
    ups = eng.gain_exp(70)
    check("累计 100 → Lv2", len(ups) == 2 and yc["level"] == 2,
          "level={} ups={}".format(yc["level"], len(ups)))
    yc_up = [u for u in ups if u["pid"] == "yinchuan"][0]
    check("升级成长 §6.4（HP+12/SP+4/攻+2/魔+2/防+1/魔防+1/速+1）",
          yc_up["gains"] == {"hp": 12, "sp": 4, "atk": 2, "def": 1,
                             "mag": 2, "mdef": 1, "spd": 1},
          str(yc_up["gains"]))
    check("升级后 HP 回满", gs.get_member("yinchuan")["hp"] == before["hp"] + 12,
          "hp={}".format(gs.get_member("yinchuan")["hp"]))


# ---------- 入口 ----------

def run_all():
    log("[verify] 阶段 3 叙事管线校验开始")
    verify_data_loader()
    verify_example_nodes()
    verify_dialogue_walkthrough()
    verify_damage_formula()
    verify_interludes_and_level()
    log("")
    log("[verify] 校验完成：失败 {} 项".format(failed))
    if failed == 0:
        log("[verify] 阶段 3 全部通过")
    else:
        log("[verify] 存在失败项，请检查上述 FAIL 输出")
    return failed


def main():
    return run_all()


if __name__ == "__main__":
    sys.exit(1 if run_all() else 0)


# ---------- py_sandbox 兼容入口 ----------

def on_init():
    run_all()


def on_update(dt):
    pass


def on_render():
    pass


def on_shutdown():
    pass
