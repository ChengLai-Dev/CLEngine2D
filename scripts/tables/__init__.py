import importlib
import os
import re


class _TableManager:
    _tables = {}

    @classmethod
    def Initialize(cls):
        cls._tables = {}
        data_dir = os.path.join(os.path.dirname(__file__), "data")
        if not os.path.isdir(data_dir):
            return
        for fname in os.listdir(data_dir):
            m = re.match(r"^([a-zA-Z]\w*)\.py$", fname)
            if m:
                name = m.group(1)
                mod = importlib.import_module(f"tables.data.{name}")
                cls._tables[name] = getattr(mod, name)

    @classmethod
    def Get(cls, name, key):
        return cls._tables[name][key]

    @classmethod
    def GetAll(cls, name):
        return cls._tables[name]


TableManager = _TableManager
