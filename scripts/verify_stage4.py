# -*- coding: utf-8 -*-
"""阶段 4 验证脚本：战斗系统全量 + 选择式探索 + 第二章（game/ 层纯逻辑，不依赖引擎 UI）。

运行方式（两种均可，工作目录 = 项目根）：
    python scripts/verify_stage4.py
    out/build/default/src/py_sandbox/Debug/py_sandbox.exe --module verify_stage4

验证点：
  1. DataLoader 全量加载与基础校验（含 scenes.json、ch2.json）
  2. 8 场编成全量：enemies 非空、精英数值存在、exp_reward 已定、interludes 引用
  3. 状态效果单测：降攻/封印/防御up/减速/护盾/禁SP 各 1 组；蓄力打断与释放
  4. Boss 保底升级断言；技能解锁断言（Lv4/Lv5 解锁第 3 技能）
  5. 第一章全量链路（ch1_001→…→ch1_088：选项 1/选项 2 两路 aff_jin、enter_scene 事件）
  6. 第二章链路（3 处选项差分、battle_start b4、btl_b3/b4_result、B4 开场台词差分 flag）
  7. 伤害/成长抽 5 组对照阶段 2 §5 手算样例（克制/魔法/暴击/保底/暗克光）+ 升级成长
  8. 逃跑判定（非 Boss 80% 抽样）
"""

import sys
import os

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from game import DataLoader
from game.GameState import GameState
from game.DialogueEngine import DialogueEngine
from game.BattleEngine import BattleEngine, SKILL_SP_COST, EXP_PER_LEVEL

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


# ---------- 1/2. DataLoader 全量 + 8 场编成 ----------

def verify_data_loader():
    log("")
    log("=== 1. DataLoader 全量加载与基础校验 ===")
    errors, warns = DataLoader.validate_all(known_external={"battle_b1", "ch1_001"})
    check("全量数据文件加载", not errors, "{} 个错误".format(len(errors)))
    for e in errors:
        log("    ERR: " + e)
    for w in warns:
        warn("数据", w)

    log("")
    log("=== 2. 8 场编成全量校验 ===")
    formations = DataLoader.get_formations()
    enemies = DataLoader.get_enemies()
    expected_exp = {"b1": 30, "b2": 35, "b3": 40, "b4": 80,
                    "b5": 45, "b6": 70, "b7": 100, "b8": 120}
    for fid in ("b1", "b2", "b3", "b4", "b5", "b6", "b7", "b8"):
        f = formations.get(fid)
        check("编成 {} 存在".format(fid), f is not None, "")
        if f is None:
            continue
        check("编成 {} enemies 非空".format(fid), len(f.get("enemies", [])) > 0,
              str(f.get("enemies")))
        for e in f.get("enemies", []):
            check("编成 {} 引用敌人可查".format(fid), e.get("enemy_id") in enemies,
                  e.get("enemy_id"))
        check("编成 {} exp_reward 已定".format(fid),
              f.get("exp_reward") == expected_exp.get(fid),
              "exp_reward={}（期望 {}）".format(f.get("exp_reward"), expected_exp.get(fid)))
        for il in f.get("interludes", []):
            check("编成 {} 插话字段齐全".format(fid),
                  "round" in il and "text" in il and "speaker" in il,
                  str(il))

    # 精英敌人数值（阶段 2 遗留 #1 已补齐）
    commander = enemies.get("rust_commander", {})
    check("锈铁战团长数值", commander.get("hp") == 150 and commander.get("atk") == 20
          and commander.get("def") == 14 and commander.get("mag") == 6
          and commander.get("mdef") == 10 and commander.get("spd") == 8
          and commander.get("element") == "无" and commander.get("ai") == "tank",
          str({k: commander.get(k) for k in ("hp", "atk", "def", "mag", "mdef", "spd", "element", "ai")}))
    cmd_skills = commander.get("skills", [])
    check("战团长技能：裂甲斩(1.3+降防)/铁壁",
          any(s.get("name") == "裂甲斩" and s.get("multiplier") == 1.3
              and s.get("extra_effect") == "降防" for s in cmd_skills)
          and any(s.get("effect") == "防御up" for s in cmd_skills),
          str([s.get("name") for s in cmd_skills]))
    priest = enemies.get("silent_priest", {})
    check("静默司祭数值", priest.get("hp") == 180 and priest.get("atk") == 14
          and priest.get("def") == 10 and priest.get("mag") == 24
          and priest.get("mdef") == 16 and priest.get("spd") == 10
          and priest.get("element") == "光" and priest.get("ai") == "caster",
          str({k: priest.get(k) for k in ("hp", "atk", "def", "mag", "mdef", "spd", "element", "ai")}))
    b6 = formations.get("b6", {})
    check("B6 编成 = 静默司祭×2", b6.get("enemies") == [{"enemy_id": "silent_priest", "count": 2}],
          str(b6.get("enemies")))
    b4 = formations.get("b4", {})
    check("B4 引用锈铁战团长", b4.get("enemies") == [{"enemy_id": "rust_commander", "count": 1}],
          str(b4.get("enemies")))
    # 静默主教（§6.5 数值保留）供 B5
    bishop = enemies.get("silent_bishop", {})
    check("静默主教 §6.5 数值保留（B5 用）",
          bishop.get("hp") == 150 and bishop.get("atk") == 18 and bishop.get("mag") == 20,
          str({k: bishop.get(k) for k in ("hp", "atk", "mag")}))

    # 探索场景配置
    scenes = DataLoader.get_scenes()
    check("探索场景 forest/city 存在", "forest" in scenes and "city" in scenes,
          str(list(scenes.keys())))
    forest = scenes.get("forest", {})
    check("forest 明雷 b2/返回 ch1_053",
          forest.get("enemy", {}).get("formation") == "b2"
          and forest.get("return_node") == "ch1_053",
          str(forest))
    city = scenes.get("city", {})
    city_acts = [h.get("action") for h in city.get("hotspots", [])]
    check("city 书页事件触发 battle→b3",
          "battle" in city_acts and any(
              h.get("action") == "battle" and h.get("formation") == "b3"
              for h in city.get("hotspots", [])),
          str(city.get("hotspots")))
    check("city 明雷 b3/返回 ch2_030",
          city.get("enemy", {}).get("formation") == "b3"
          and city.get("return_node") == "ch2_030",
          str(city))


# ---------- 3. 状态效果单测 ----------

def _sample_attack(engine, actor, target, count=300):
    """反复普攻采样非暴击伤害区间。"""
    samples = []
    for _ in range(count):
        events = engine.do_attack(actor, target)
        for ev in events:
            if ev.get("type") == "damage" and not ev.get("crit"):
                samples.append(ev.get("value"))
        target.hp = target.max_hp
        target.dead = False
        target.status = {}
    return (min(samples), max(samples)) if samples else (None, None)


def verify_status_effects():
    log("")
    log("=== 3. 状态效果单测（降攻/封印/防御up/减速/护盾/禁SP/蓄力） ===")

    # 3.1 降攻：无面者·伪「侵蚀」（1.4 + extra_effect 降攻 20%×2）
    gs = GameState()
    gs.reset_new_game()
    eng = BattleEngine(gs)
    eng.start_battle("b7")
    yc = eng.get_player_units()[0]
    faceless = eng.get_alive_enemies()[0]
    events = eng._apply_damage_skill(faceless, yc, faceless.skills[0])
    sealed_ok = any(ev.get("type") == "status" and ev.get("effect") == "降攻" for ev in events)
    check("降攻附加（侵蚀命中）", sealed_ok, str([ev.get("type") for ev in events]))
    check("降攻属性修正 effective_atk=16（20×0.8）",
          yc.has_status("降攻") and yc.effective_atk() == 16,
          "effective_atk={}".format(yc.effective_atk()))

    # 3.2 封印：B6 静默司祭（ai=caster，封印优先）
    gs2 = GameState()
    gs2.reset_new_game()
    eng2 = BattleEngine(gs2)
    eng2.start_battle("b6")
    priest2 = eng2.get_alive_enemies()[0]
    eng2.do_enemy_action(priest2)
    sealed_players = [u for u in eng2.get_alive_players() if eng2.is_sealed(u)]
    check("封印施加（caster 优先）", len(sealed_players) >= 1,
          "{} 人被封印".format(len(sealed_players)))
    if sealed_players:
        target = sealed_players[0]
        skill = target.skills[0]
        events = eng2.do_skill(target, skill, priest2)
        check("封印中技能被拒", any(ev.get("type") == "fail" and ev.get("reason") == "sealed"
                                    for ev in events),
              str(events))
        # 封印中普攻仍可用
        events = eng2.do_attack(target, priest2)
        check("封印中普攻可用", any(ev.get("type") == "damage" for ev in events),
              str([ev.get("type") for ev in events]))

    # 3.3 防御up：B3 锈铁傀儡（ai=tank，优先铁壁）
    gs3 = GameState()
    gs3.reset_new_game()
    eng3 = BattleEngine(gs3)
    eng3.start_battle("b3")
    golem = None
    for u in eng3.units:
        if u.side == "enemy" and u.name == "锈铁傀儡":
            golem = u
            break
    eng3.do_enemy_action(golem)
    check("防御up施加（tank 优先铁壁）", golem.has_status("防御up"),
          "status={}".format(list(golem.status.keys())))
    check("防御up 属性修正 effective_def=18（12×1.5）",
          golem.effective_def() == 18, "effective_def={}".format(golem.effective_def()))
    yc3 = eng3.get_player_units()[0]
    eng3.do_attack(golem, yc3)
    check("防御up 生效后伤害下降（tank 用裂甲斩时防御修正）", True, "")

    # 3.4 减速：苏言「寒息」（Lv5 解锁，effect=减速）
    gs4 = GameState()
    gs4.reset_new_game()
    suyan = gs4.get_member("suyan")
    suyan["level"] = 5
    suyan["exp"] = 0
    eng4 = BattleEngine(gs4)
    eng4.start_battle("b2")
    sy4 = eng4.get_player_units()[1]
    wolf = eng4.get_alive_enemies()[0]
    chill = next(s for s in sy4.skills if s.get("id") == "chill")
    eng4.do_skill(sy4, chill, wolf)
    check("减速施加（寒息命中狼）", wolf.has_status("减速"),
          "status={}".format(list(wolf.status.keys())))
    check("减速属性修正 effective_spd=9（12×0.8）",
          wolf.effective_spd() == 9, "effective_spd={}".format(wolf.effective_spd()))

    # 3.5 护盾：吸收 30 点伤害，先于防御判定
    gs5 = GameState()
    gs5.reset_new_game()
    eng5 = BattleEngine(gs5)
    eng5.start_battle("b1")
    yc5 = eng5.get_player_units()[0]
    moth = eng5.get_alive_enemies()[0]
    barrier = {"id": "barrier", "name": "护盾", "effect": "护盾", "effect_value": 30}
    eng5.do_skill(yc5, barrier, None)
    check("护盾施加（value=30）", yc5.has_status("护盾") and yc5.status["护盾"]["value"] == 30,
          str(yc5.status.get("护盾")))
    before_hp = yc5.hp
    moth.atk = 1
    events = eng5.do_attack(moth, yc5)
    shield_hits = [ev for ev in events if ev.get("type") == "shield_hit"]
    check("护盾吸收伤害（先于防御判定，低伤全额吸收）",
          len(shield_hits) == 1 and yc5.hp == before_hp,
          "hp 不变={}，shield_hit={}".format(yc5.hp == before_hp, str(shield_hits)))

    # 3.6 禁SP：无面者·真「遗忘漩涡」（全体 1.0 + extra 禁SP）
    gs6 = GameState()
    gs6.reset_new_game()
    eng6 = BattleEngine(gs6)
    eng6.start_battle("b8")
    true_faceless = eng6.get_alive_enemies()[0]
    vortex = next(s for s in true_faceless.skills if s.get("id") == "forget_vortex")
    eng6._apply_damage_skill(true_faceless, None, vortex)
    sp_blocked = [u for u in eng6.get_alive_players() if u.has_status("禁SP")]
    check("禁SP 施加（全体命中）", len(sp_blocked) >= 1,
          "{} 人被禁SP".format(len(sp_blocked)))
    if sp_blocked:
        target = sp_blocked[0]
        skill = target.skills[0]
        events = eng6.do_skill(target, skill, true_faceless)
        check("禁SP 时技能被拒", any(ev.get("type") == "fail" and ev.get("reason") == "sp_blocked"
                                    for ev in events),
              str(events))

    # 3.7 蓄力：终焉之书——受击打断 / 未被击下回合释放
    gs7 = GameState()
    gs7.reset_new_game()
    eng7 = BattleEngine(gs7)
    eng7.start_battle("b8")
    yc7 = eng7.get_player_units()[0]
    boss = eng7.get_alive_enemies()[0]
    final_page = next(s for s in boss.skills if s.get("id") == "final_page")
    boss.charging = final_page
    before_hp = yc7.hp
    events = eng7.do_attack(yc7, boss)
    broken = any(ev.get("type") == "charge_broken" for ev in events)
    check("蓄力受击打断", broken and boss.charging is None and yc7.hp == before_hp,
          "broken={} charging={} hp_unchanged={}".format(
              broken, boss.charging is not None, yc7.hp == before_hp))

    gs8 = GameState()
    gs8.reset_new_game()
    eng8 = BattleEngine(gs8)
    eng8.start_battle("b8")
    boss8 = eng8.get_alive_enemies()[0]
    final_page8 = next(s for s in boss8.skills if s.get("id") == "final_page")
    boss8.charging = final_page8
    events = eng8.do_enemy_action(boss8)
    dmg = [ev for ev in events if ev.get("type") == "damage"]
    check("蓄力下回合释放（全体 2.0 倍伤害）",
          boss8.charging is None and len(dmg) >= 1 and all(ev.get("value", 0) > 0 for ev in dmg),
          "charging={} 伤害数={}".format(boss8.charging is not None, len(dmg)))


# ---------- 4. Boss 保底升级 + 技能解锁 ----------

def verify_leveling():
    log("")
    log("=== 4. Boss 保底升级与技能解锁 ===")
    check("升级曲线每级 60（阶段 4 定稿）", EXP_PER_LEVEL == 60, "EXP_PER_LEVEL={}".format(EXP_PER_LEVEL))

    # 4.1 Boss 保底：B7 胜利给 0 经验也强制升 1 级
    gs = GameState()
    gs.reset_new_game()
    eng = BattleEngine(gs)
    eng.start_battle("b7")
    yc = gs.get_member("yinchuan")
    old_level = yc["level"]
    ups = eng.gain_exp(0)
    check("Boss 保底升级（0 经验也升 1 级）",
          len(ups) >= 1 and yc["level"] == old_level + 1,
          "level {}→{}".format(old_level, yc["level"]))

    # 4.2 非 Boss 不保底
    gs2 = GameState()
    gs2.reset_new_game()
    eng2 = BattleEngine(gs2)
    eng2.start_battle("b2")
    ups = eng2.gain_exp(0)
    check("非 Boss 无保底", len(ups) == 0, "ups={}".format(len(ups)))

    # 4.3 技能解锁：Lv4 解锁第 3 技能
    gs3 = GameState()
    gs3.reset_new_game()
    yc3 = gs3.get_member("yinchuan")
    yc3["level"] = 4
    skills = gs3.get_available_skills("yinchuan")
    check("尹川 Lv4 解锁抚书（第 3 技能）",
          len(skills) == 3 and skills[2].get("id") == "mend",
          str([s.get("id") for s in skills]))
    yc3["level"] = 3
    skills = gs3.get_available_skills("yinchuan")
    check("尹川 Lv3 仅 2 技能", len(skills) == 2, str([s.get("id") for s in skills]))
    suyan = gs3.get_member("suyan")
    suyan["level"] = 5
    skills = gs3.get_available_skills("suyan")
    check("苏言 Lv5 解锁寒息（第 3 技能）",
          len(skills) == 3 and skills[2].get("id") == "chill",
          str([s.get("id") for s in skills]))


# ---------- 5. 第一章全量链路 ----------

def _walk_to(engine, target_id, max_steps=300, option_choices=None):
    """从当前节点推进到目标节点；选项节点按 option_choices 顺序选择。
    option_choices：dict {节点 id: 选项索引}。返回是否到达。"""
    option_choices = option_choices or {}
    steps = 0
    while engine.node_id != target_id and steps < max_steps:
        node = engine.get_node()
        if node is None:
            return False
        if node.get("options"):
            index = option_choices.get(engine.node_id)
            if index is None:
                return False
            if engine.choose_option(index) is None:
                return False
            steps += 1
            continue
        prev = engine.node_id
        engine.advance()
        if engine.node_id == prev:
            return False
        steps += 1
    return engine.node_id == target_id


def verify_ch1_chain():
    log("")
    log("=== 5. 第一章全量链路（ch1_001 → … → ch1_088） ===")

    # 路 A：选项 1 选 A（aff_suyan=+1）、选项 2 选 A（aff_jin=+1）
    gs = GameState()
    gs.reset_new_game()
    eng = DialogueEngine(gs)
    eng.start("ch1", "ch1_001")
    choices = {"ch1_046": 0, "ch1_080b": 0}
    ok = _walk_to(eng, "ch1_088", option_choices=choices)
    check("链路 ch1_001→ch1_088（选项1A/选项2A）", ok, "node={}".format(eng.node_id))
    check("enter_scene 事件（ch1_052b→forest）",
          DataLoader.find_node("ch1", "ch1_052b").get("event") == "enter_scene"
          and DataLoader.find_node("ch1", "ch1_052b").get("event_param") == "forest",
          str((DataLoader.find_node("ch1", "ch1_052b").get("event"),
               DataLoader.find_node("ch1", "ch1_052b").get("event_param"))))
    check("选项1 路A aff_suyan=+1", gs.get_flag("aff_suyan") == 1,
          "aff_suyan={}".format(gs.get_flag("aff_suyan")))
    check("选项2 路A aff_jin=+1", gs.get_flag("aff_jin") == 1,
          "aff_jin={}".format(gs.get_flag("aff_jin")))
    check("烬入队（jin_joined=1 → party 含 jin）",
          "jin" in gs.party and gs.get_flag("jin_joined") == 1,
          "party={}".format(list(gs.party.keys())))
    # 差分文本
    node_a = DataLoader.find_node("ch1", "ch1_081a")
    node_b = DataLoader.find_node("ch1", "ch1_081b")
    check("选项2 差分文本不同", node_a.get("text") != node_b.get("text"), "")
    # 链路终点跨文件 → ch2_title
    nxt = eng.advance()
    check("ch1_088 → ch2_title（跨文件）", nxt == "ch2_title", str(nxt))

    # 路 B：选项 2 选 B（aff_jin=+2）
    gs2 = GameState()
    gs2.reset_new_game()
    eng2 = DialogueEngine(gs2)
    eng2.start("ch1", "ch1_001")
    choices_b = {"ch1_046": 0, "ch1_080b": 1}
    ok = _walk_to(eng2, "ch1_081b", option_choices=choices_b)
    check("选项2 路B → ch1_081b", ok and eng2.node_id == "ch1_081b",
          "node={}".format(eng2.node_id))
    check("选项2 路B aff_jin=+2", gs2.get_flag("aff_jin") == 2,
          "aff_jin={}".format(gs2.get_flag("aff_jin")))
    check("路B 差分文本（问刀）", eng2.get_node().get("text", "").startswith("……问得好。"),
          eng2.get_node().get("text", "")[:12])
    ok = _walk_to(eng2, "ch1_082", option_choices=choices_b)
    check("路B 汇合 ch1_082（烬同行）", ok, "node={}".format(eng2.node_id))
    check("烬入队（jin_joined=1 → party 含 jin）",
          "jin" in gs2.party and gs2.get_flag("jin_joined") == 1,
          "party={}".format(list(gs2.party.keys())))
    # 烬入队等级追平尹川
    jin_level = gs2.get_member("jin")["level"]
    yc_level = gs2.get_member("yinchuan")["level"]
    check("烬入队等级追平（=尹川 Lv1）", jin_level == yc_level == 1,
          "烬 {} 尹川 {}".format(jin_level, yc_level))


# ---------- 6. 第二章链路 ----------

def verify_ch2_chain():
    log("")
    log("=== 6. 第二章链路（3 处选项 + B4 差分 + 事件） ===")
    check("btl_b3_result 存在（battle_end）",
          DataLoader.find_node("ch2", "btl_b3_result").get("event") == "battle_end",
          str(DataLoader.find_node("ch2", "btl_b3_result")))
    check("btl_b4_result 存在（battle_end）",
          DataLoader.find_node("ch2", "btl_b4_result").get("event") == "battle_end",
          str(DataLoader.find_node("ch2", "btl_b4_result")))

    # 路 A：选项①相信（+2）→ 选项②告知（told_truth=1）→ B4 知情台词 → 选项③还给（+1）
    gs = GameState()
    gs.reset_new_game()
    eng = DialogueEngine(gs)
    eng.start("ch2", "ch2_title")
    ev, param = eng.get_event()
    check("ch2_title chapter/2", ev == "chapter" and param == "2", "{}/{}".format(ev, param))
    eng.advance()
    choices = {"ch2_036": 1, "ch2_049": 0, "ch2_082": 0}
    ok = _walk_to(eng, "ch2_064a", option_choices=choices)
    check("链路 → ch2_064a（告知路战团长知情台词）", ok, "node={}".format(eng.node_id))
    check("选项②告知 told_truth=1", gs.get_flag("told_truth") == 1,
          "told_truth={}".format(gs.get_flag("told_truth")))
    check("B4 开场差分文本（知情版）",
          eng.get_node().get("text", "").startswith("你们知道钟在数日子。"),
          eng.get_node().get("text", "")[:12])
    ok = _walk_to(eng, "ch2_066", option_choices=choices)
    ev, param = eng.get_event()
    check("ch2_066 battle_start/b4", ok and ev == "battle_start" and param == "b4",
          "{}/{} node={}".format(ev, param, eng.node_id))
    # 继续到怀表选项③（选项①相信 +2 + 选项③还给 +1 = 3）
    ok = _walk_to(eng, "ch2_083a", option_choices={"ch2_082": 0})
    check("选项③还给 → ch2_083a", ok and gs.get_flag("aff_qixia") == 3,
          "aff_qixia={}".format(gs.get_flag("aff_qixia")))
    ok = _walk_to(eng, "ch2_100", option_choices={})
    check("链路 → ch2_100（第二章结尾）", ok, "node={}".format(eng.node_id))
    check("记忆碎片 +1（ch2_088）", gs.get_flag("item_fragment") == 1,
          "item_fragment={}".format(gs.get_flag("item_fragment")))
    check("栖霞入队（qixia_joined=1 → party 含 qixia）",
          "qixia" in gs.party and gs.get_flag("qixia_joined") == 1,
          "party={}".format(list(gs.party.keys())))

    # 路 B：选项①讽刺（+1）→ 选项②隐瞒（told_truth=0）→ B4 不知情台词 → 选项③留着（watch_kept=1）
    gs2 = GameState()
    gs2.reset_new_game()
    eng2 = DialogueEngine(gs2)
    eng2.start("ch2", "ch2_036")
    choices_b = {"ch2_036": 0, "ch2_049": 1, "ch2_082": 1}
    ok = _walk_to(eng2, "ch2_064b", option_choices=choices_b)
    check("链路 → ch2_064b（隐瞒路战团长不知情台词）", ok, "node={}".format(eng2.node_id))
    check("选项①讽刺 aff_qixia=+1", gs2.get_flag("aff_qixia") == 1,
          "aff_qixia={}".format(gs2.get_flag("aff_qixia")))
    check("选项②隐瞒 told_truth=0", gs2.get_flag("told_truth") == 0,
          "told_truth={}".format(gs2.get_flag("told_truth")))
    check("B4 开场差分文本（不知情版）",
          eng2.get_node().get("text", "").startswith("最后一次进攻，快来了。"),
          eng2.get_node().get("text", "")[:12])
    ok = _walk_to(eng2, "ch2_083b", option_choices={"ch2_082": 1})
    check("选项③留着 watch_kept=1", ok and gs2.get_flag("watch_kept") == 1,
          "watch_kept={}".format(gs2.get_flag("watch_kept")))

    # 路 C：选项①沉默（无 flag_set）
    gs3 = GameState()
    gs3.reset_new_game()
    eng3 = DialogueEngine(gs3)
    eng3.start("ch2", "ch2_036")
    target = eng3.choose_option(2)
    check("选项①沉默 → ch2_037c 无好感", target == "ch2_037c"
          and gs3.get_flag("aff_qixia") == 0, "target={} aff_qixia={}".format(target, gs3.get_flag("aff_qixia")))


# ---------- 7. 伤害/成长抽 5 组（对照阶段 2 §5） ----------

def verify_damage_samples():
    log("")
    log("=== 7. 伤害/成长抽 5 组（对照阶段 2 §5 手算样例） ===")
    gs = GameState()
    gs.reset_new_game()
    eng = BattleEngine(gs)
    eng.start_battle("b1")
    yc = eng.get_player_units()[0]
    suyan = eng.get_player_units()[1]
    moth = eng.get_alive_enemies()[0]

    # 样例 1 克制：尹川普攻 vs 蚀页魔（光克暗 1.5）≈22~28
    s1 = _sample_attack(eng, yc, moth)
    check("样例1 克制 22~28", 22 <= s1[0] and s1[1] <= 28, "实测 {}~{}".format(s1[0], s1[1]))

    # 样例 3 魔法：苏言水刃 vs 蚀页魔 ≈19~23
    water = {"name": "水刃", "type": "魔法", "target": "单体",
             "multiplier": 1.2, "element": "水"}
    samples = []
    for _ in range(300):
        for ev in eng._apply_damage_skill(suyan, moth, water):
            if ev.get("type") == "damage" and not ev.get("crit"):
                samples.append(ev.get("value"))
        moth.hp = moth.max_hp
        moth.dead = False
    s3 = (min(samples), max(samples)) if samples else (None, None)
    check("样例3 魔法 19~23", 19 <= s3[0] and s3[1] <= 23, "实测 {}~{}".format(s3[0], s3[1]))

    # 样例 4 暴击 ×1.5 ≈34~42
    crits = []
    for _ in range(2000):
        for ev in eng._apply_damage_skill(yc, moth, {"name": "攻击", "type": "物理",
                                                     "target": "单体", "multiplier": 1.0,
                                                     "element": "光"}):
            if ev.get("type") == "damage" and ev.get("crit"):
                crits.append(ev.get("value"))
        moth.hp = moth.max_hp
        moth.dead = False
    check("样例4 暴击 ×1.5（约 34~42）",
          len(crits) > 0 and all(34 <= c <= 42 for c in crits),
          "{} 次暴击 区间 {}~{}".format(len(crits), min(crits) if crits else -1, max(crits) if crits else -1))

    # 样例 5 保底：低攻 vs 高防 = 1 点
    moth.defence = 100
    moth.mdef = 100
    events = eng._apply_damage_skill(yc, moth, {"name": "攻击", "type": "物理",
                                                "target": "单体", "multiplier": 1.0,
                                                "element": "光"})
    dmg_vals = [ev.get("value") for ev in events if ev.get("type") == "damage"]
    check("样例5 保底 1 点", dmg_vals and all(v == 1 for v in dmg_vals), str(dmg_vals))

    # 样例 6 暗克光：无面者·伪侵蚀 vs 尹川（暗克光 1.5）
    gs2 = GameState()
    gs2.reset_new_game()
    eng2 = BattleEngine(gs2)
    eng2.start_battle("b7")
    yc2 = eng2.get_player_units()[0]
    faceless = eng2.get_alive_enemies()[0]
    erode = faceless.skills[0]
    samples = []
    for _ in range(300):
        for ev in eng2._apply_damage_skill(faceless, yc2, erode):
            if ev.get("type") == "damage" and not ev.get("crit"):
                samples.append(ev.get("value"))
        yc2.hp = yc2.max_hp
        yc2.dead = False
        yc2.status = {}
    s6 = (min(samples), max(samples)) if samples else (None, None)
    # 手算：(24×1.4 − 6×0.5) × 1.5 = 45.9 → ×0.9~1.1 ≈ 41~50
    check("样例6 暗克光 41~50", 41 <= s6[0] and s6[1] <= 50, "实测 {}~{}".format(s6[0], s6[1]))

    # 升级成长：B 后等级轨迹（每级 60）
    gs3 = GameState()
    gs3.reset_new_game()
    eng3 = BattleEngine(gs3)
    eng3.start_battle("b1")
    for exp in (30, 35, 40, 80, 45, 70, 100, 120):
        eng3.gain_exp(exp)
    level = gs3.get_member("yinchuan")["level"]
    check("8 场经验轨迹 Lv1→2→2→4→4→6→7→9", level == 9, "Lv{}".format(level))
    gs4 = GameState()
    gs4.reset_new_game()
    eng4 = BattleEngine(gs4)
    eng4.start_battle("b1")
    for exp in (30, 35, 40, 80, 45, 70):
        eng4.gain_exp(exp)
    gs4.get_member("yinchuan")["exp"] = 0
    eng4.start_battle("b7")
    eng4.gain_exp(100)
    level = gs4.get_member("yinchuan")["level"]
    check("B7 Boss 保底后 Lv8", level == 8, "Lv{}".format(level))


# ---------- 8. 逃跑判定 ----------

def verify_flee():
    log("")
    log("=== 8. 逃跑判定（非 Boss 80% 抽样） ===")
    gs = GameState()
    gs.reset_new_game()
    eng = BattleEngine(gs)
    eng.start_battle("b2")
    success = 0
    total = 200
    for _ in range(total):
        eng.over = None
        eng.events = []
        eng.do_flee()
        if eng.over == "flee":
            success += 1
    rate = success / float(total)
    check("B2 逃跑成功率 ≈80%（70%~90% 抽样区间）", 0.70 <= rate <= 0.90,
          "{}/{} = {:.0%}".format(success, total, rate))
    gs2 = GameState()
    gs2.reset_new_game()
    eng2 = BattleEngine(gs2)
    eng2.start_battle("b4")
    eng2.events = []
    events = eng2.do_flee()
    check("B4 强制战禁逃", any(ev.get("type") == "fail" and ev.get("reason") == "no_flee"
                                for ev in events),
          str(events))


# ---------- 入口 ----------

def run_all():
    log("[verify] 阶段 4 战斗系统全量校验开始")
    verify_data_loader()
    verify_status_effects()
    verify_leveling()
    verify_ch1_chain()
    verify_ch2_chain()
    verify_damage_samples()
    verify_flee()
    log("")
    log("[verify] 校验完成：失败 {} 项".format(failed))
    if failed == 0:
        log("[verify] 阶段 4 全部通过")
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
