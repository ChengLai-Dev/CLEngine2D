# -*- coding: utf-8 -*-
"""打字机组件：逐字驱动 Label.SetText。

- 速度：字/秒（cps）；0 = 整句即时显示
- skip()：跳完整句（D-09：打字中按推进键直接跳句）
- 完成回调 on_done
- 实际速度由调用方传入（节点 typewriter 优先，缺省 Settings 档位，阶段 5 前默认中速 30）
"""


class Typewriter:

    def __init__(self, label):
        self.label = label
        self.text = ""
        self.cps = 30.0
        self._index = 0
        self._accum = 0.0
        self.done = True
        self.on_done = None

    def start(self, text, cps, on_done=None):
        """开始逐字输出；cps<=0 或空文本 = 整句即时显示。"""
        self.text = text or ""
        self.cps = cps if cps and cps > 0 else 0.0
        self._index = 0
        self._accum = 0.0
        self.done = False
        self.on_done = on_done
        if self.cps <= 0.0 or not self.text:
            self._finish()
            return
        self.label.SetText(self._current())

    def update(self, dt):
        if self.done:
            return
        if dt <= 0.0:
            return
        self._accum += dt
        interval = 1.0 / self.cps
        while self._accum >= interval and not self.done:
            self._accum -= interval
            self._index += 1
            if self._index >= len(self.text):
                self._finish()
                return
            self.label.SetText(self._current())

    def skip(self):
        """直接显示完整文本（D-09：打字中按推进键跳完整句）。"""
        if not self.done:
            self._finish()

    def is_typing(self):
        return not self.done

    def _current(self):
        return self.text[:self._index + 1]

    def _finish(self):
        self.label.SetText(self.text)
        self.done = True
        if self.on_done is not None:
            self.on_done()
