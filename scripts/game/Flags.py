# -*- coding: utf-8 -*-
"""Flag 系统：条件求值与赋值（纯逻辑，不依赖引擎）。

条件表达式（flag_require）：`aff_suyan>=2`、`visited_b2=1`、`aff_suyan>=2&&aff_jin>=5`
赋值表达式（flag_set / 选项 flag_set）：`aff_suyan=+1`（相对增减）、`visited_b2=1`（绝对值）、
`aff_jin=+1;visited_b4=1`（分号组合）
"""

import re

_OP_RE = re.compile(r"^([a-z0-9_]+)\s*(>=|<=|>|<|=)\s*(-?\d+)$")
_SET_ITEM_RE = re.compile(r"^([a-z0-9_]+)=([+-]?\d+)$")

OPS = {
    ">=": lambda a, b: a >= b,
    "<=": lambda a, b: a <= b,
    ">": lambda a, b: a > b,
    "<": lambda a, b: a < b,
    "=": lambda a, b: a == b,
}


def eval_require(expr, flags):
    """求值条件表达式；表达式为空或非法一律返回 False。"""
    if not expr:
        return False
    parts = [p.strip() for p in expr.split("&&")]
    for part in parts:
        m = _OP_RE.match(part)
        if not m:
            return False
        name, op, value = m.group(1), m.group(2), int(m.group(3))
        current = flags.get(name, 0)
        if not OPS[op](current, value):
            return False
    return True


def apply_set(expr, flags):
    """应用赋值表达式（就地修改 flags）。非法表达式静默忽略。"""
    if not expr:
        return
    for item in expr.split(";"):
        item = item.strip()
        m = _SET_ITEM_RE.match(item)
        if not m:
            continue
        name, value_str = m.group(1), m.group(2)
        if value_str.startswith("+") or value_str.startswith("-"):
            delta = int(value_str)
            flags[name] = flags.get(name, 0) + delta
        else:
            flags[name] = int(value_str)
