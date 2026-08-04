# -*- coding: utf-8 -*-
"""缓动组件：透明度/位移/缩放/旋转/宽度(血条)的逐帧插值。

全部动画统一走本组件（方案文档 §3.1：不写手撸逐帧循环）。
每个场景控制器持有自己的 Tweener 实例；on_exit 时必须 clear()
（场景切换后旧节点引用悬垂，不得再触碰）。
"""

from CLEngine.Math import Vec2, Vec3


def _ease_linear(t):
    return t


def _ease_quad_in(t):
    return t * t


def _ease_quad_out(t):
    return 1.0 - (1.0 - t) * (1.0 - t)


def _ease_sine_out(t):
    import math
    return math.sin(t * math.pi * 0.5)


_EASES = {
    "linear": _ease_linear,
    "quad_in": _ease_quad_in,
    "quad_out": _ease_quad_out,
    "sine_out": _ease_sine_out,
}


class _Tween:
    """单个缓动：attr 支持 position/opacity/scale/rotation/width。"""

    def __init__(self, node, attr, end, dur, start, ease, on_finish, delay, pingpong, on_cycle):
        self.node = node
        self.attr = attr
        self.end = end
        self.dur = dur if dur > 0 else 0.001
        self.start = start
        self.ease = ease
        self.on_finish = on_finish
        self.on_cycle = on_cycle
        self.delay = delay
        self.elapsed = -delay
        self.pingpong = pingpong
        self.done = False

    def _read(self):
        node = self.node
        if self.attr == "position":
            return node.GetPosition()
        if self.attr == "opacity":
            return node.GetOpacity()
        if self.attr == "scale":
            return node.GetScale()
        if self.attr == "rotation":
            return node.GetRotationZ()
        if self.attr == "width":
            return node.GetContentSize().x
        return 0.0

    def _write(self, value):
        node = self.node
        if self.attr == "position":
            node.SetPosition(value)
        elif self.attr == "opacity":
            node.SetOpacity(value)
        elif self.attr == "scale":
            node.SetScale(value)
        elif self.attr == "rotation":
            node.SetRotationZ(value)
        elif self.attr == "width":
            size = node.GetContentSize()
            node.SetContentSize(Vec2(value, size.y))

    def _lerp(self, t):
        a = self.start
        b = self.end
        if self.attr == "position":
            return Vec3(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t)
        if self.attr == "scale":
            return Vec3(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t)
        return a + (b - a) * t

    def update(self, dt):
        if self.done:
            return
        self.elapsed += dt
        if self.elapsed < 0:
            return
        t = self.elapsed / self.dur
        if t >= 1.0:
            self._write(self.end)
            if self.pingpong:
                # 往返动画：到达端点后反向；on_finish 仅"完成"时触发（pingpong 不完成），
                # 每周期回调走 on_cycle
                self.start, self.end = self.end, self.start
                self.elapsed = 0.0
                if self.on_cycle is not None:
                    self.on_cycle()
                return
            self.done = True
            if self.on_finish is not None:
                self.on_finish()
            return
        eased = _EASES.get(self.ease, _ease_linear)(t)
        self._write(self._lerp(eased))


class Tweener:
    """逐帧缓动管理器（每控制器一个实例，on_exit 清理）。"""

    def __init__(self):
        self._tweens = []

    def to(self, node, attr, end, dur=0.5, start=None, ease="linear",
           on_finish=None, delay=0.0, pingpong=False, on_cycle=None):
        """向 end 插值；start 缺省取节点当前值。

        pingpong=True 时到达端点后反向往返（永不完成，on_finish 不触发）；
        每到达一次端点触发 on_cycle。
        """
        if start is None:
            start = self._read(node, attr)
        self._tweens.append(_Tween(node, attr, end, dur, start, ease,
                                   on_finish, delay, pingpong, on_cycle))

    def _read(self, node, attr):
        if attr == "position":
            return node.GetPosition()
        if attr == "opacity":
            return node.GetOpacity()
        if attr == "scale":
            return node.GetScale()
        if attr == "rotation":
            return node.GetRotationZ()
        if attr == "width":
            return node.GetContentSize().x
        return 0.0

    def update(self, dt):
        for tween in self._tweens:
            tween.update(dt)
        self._tweens = [t for t in self._tweens if not t.done]

    def clear(self):
        """丢弃全部缓动（场景切换时必须调用，节点引用将悬垂）。"""
        self._tweens = []

    def active_count(self):
        return len(self._tweens)
