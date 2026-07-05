import os


class TableDef:
    def __init__(self, source, key, fields, name=None, sheet=None):
        self.source = source
        self.key = key
        self.fields = fields
        self.sheet = sheet
        self._validators = []
        if name:
            self.name = name
        else:
            stem = os.path.splitext(os.path.basename(source))[0]
            self.name = stem

    def validate(self, func):
        self._validators.append(func)
        return func
