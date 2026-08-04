# -*- coding: utf-8 -*-
"""对话推进纯逻辑（方案文档 §3.2 / §5.2、策划案 §5）。

职责：
- 节点查询（跨文件）
- flag_require 条件进入检查
- next 推进（显式 next / options / 数组顺序）
- 选项选择（应用 flag_set 并跳转）
- 事件暴露（battle_start / battle_end / chapter / ending / enter_scene）

不依赖引擎 UI；DialogueEngine 只维护"当前文件 + 当前节点"，
场景层负责把节点内容渲染出来并分发事件。
"""

from game import DataLoader


class DialogueEngine:

    def __init__(self, game_state):
        self.game_state = game_state
        self.file = None
        self.node_id = None

    # ---------- 状态查询 ----------

    def get_node(self):
        """当前节点 dict；不存在返回 None。"""
        if self.node_id is None:
            return None
        return DataLoader.find_node(self.file, self.node_id)

    def has_options(self):
        node = self.get_node()
        return bool(node and node.get("options"))

    def get_options(self):
        node = self.get_node()
        if not node:
            return []
        return node.get("options") or []

    def get_event(self):
        """当前节点事件 (event, event_param)；无事件返回 (None, None)。"""
        node = self.get_node()
        if not node:
            return None, None
        return node.get("event"), node.get("event_param")

    # ---------- 流程控制 ----------

    def start(self, file, node_id):
        """进入文件某节点（入口）；应用节点 flag_set。"""
        self.file = file
        self.node_id = node_id
        node = self.get_node()
        if node is not None:
            self.game_state.apply_flag_set(node.get("flag_set"))
        self.game_state.set_dialogue_context(file, node_id)

    def can_enter(self, node):
        """flag_require 进入条件检查。"""
        if node is None:
            return False
        return self.game_state.eval_flag_require(node.get("flag_require"))

    def advance(self):
        """无选项时推进到下一节点：显式 next 优先，其次数组顺序。"""
        node = self.get_node()
        if node is None:
            return None
        if node.get("options"):
            return None
        target = self._resolve_next(node)
        return self._jump(target)

    def choose_option(self, index):
        """选择选项 index：应用选项 flag_set 并跳转；非法索引返回 None。"""
        node = self.get_node()
        if node is None:
            return None
        options = node.get("options") or []
        if not (0 <= index < len(options)):
            return None
        opt = options[index]
        self.game_state.apply_flag_set(opt.get("flag_set"))
        return self._jump(opt.get("next"))

    # ---------- 内部 ----------

    def _resolve_next(self, node):
        if node.get("next"):
            return node["next"]
        # 数组顺序：找当前节点在 nodes 列表中的位置，取下一项
        data = DataLoader.get_dialogue(self.file)
        if data is None:
            return None
        nodes = data.get("nodes", [])
        for i, n in enumerate(nodes):
            if n.get("id") == self.node_id:
                if i + 1 < len(nodes):
                    return nodes[i + 1].get("id")
                return None
        return None

    def _jump(self, target):
        if target is None:
            return None
        self.node_id = target
        node = self.get_node()
        if node is not None:
            self.game_state.apply_flag_set(node.get("flag_set"))
        self.game_state.set_dialogue_context(self.file, target)
        return target
