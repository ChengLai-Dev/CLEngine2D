import sys
import types
import importlib
import os


def _get_project_modules(script_dir, main_module_name="main"):
    script_dir = os.path.normpath(os.path.abspath(script_dir))
    modules = []
    for mod_name, mod in list(sys.modules.items()):
        if mod is None:
            continue
        mod_file = getattr(mod, '__file__', None)
        if mod_file is None:
            continue
        if not mod_file.endswith('.py'):
            continue
        mod_file = os.path.normpath(os.path.abspath(mod_file))
        if mod_file.startswith(script_dir):
            if mod_name != 'engine_hotreload':
                modules.append(mod_name)
    modules.sort(key=lambda n: (n.count('.'), n == main_module_name), reverse=True)
    return modules


def _build_class_map(old_modules):
    class_map = {}
    for mod_name, old_mod in old_modules.items():
        new_mod = sys.modules.get(mod_name)
        if new_mod is None or old_mod is new_mod:
            continue
        for attr_name in dir(new_mod):
            if attr_name.startswith('_'):
                continue
            old_cls = getattr(old_mod, attr_name, None)
            new_cls = getattr(new_mod, attr_name, None)
            if isinstance(old_cls, type) and isinstance(new_cls, type) and old_cls is not new_cls:
                class_map[id(old_cls)] = new_cls
    return class_map


def _reclass_value(value, class_map, visited=None):
    if value is None:
        return
    if isinstance(value, (int, float, str, bool, bytes, bytearray)):
        return
    if visited is None:
        visited = set()
    obj_id = id(value)
    if obj_id in visited:
        return
    visited.add(obj_id)

    old_cls = type(value)
    if id(old_cls) in class_map:
        value.__class__ = class_map[id(old_cls)]

    if isinstance(value, (list, tuple)):
        for item in value:
            _reclass_value(item, class_map, visited)
    elif isinstance(value, dict):
        for v in value.values():
            _reclass_value(v, class_map, visited)
    elif isinstance(value, set):
        for item in value:
            _reclass_value(item, class_map, visited)


def perform_reload(script_dir, main_module_name="main"):
    main_mod = sys.modules.get(main_module_name)
    if main_mod is None:
        print(f"[hotreload] Module '{main_module_name}' not found")
        return False

    # Snapshot old modules (before reload)
    old_modules = {}
    for mod_name in _get_project_modules(script_dir, main_module_name):
        mod = sys.modules.get(mod_name)
        if mod is not None:
            old_modules[mod_name] = mod

    # Snapshot runtime state from main module — save everything
    # except functions, classes, and modules
    old_globals = {}
    for key, value in list(main_mod.__dict__.items()):
        if isinstance(value, (
            types.FunctionType, types.BuiltinFunctionType,
            types.ModuleType, type
        )):
            continue
        old_globals[key] = value

    # Reload all project modules (deepest first)
    project_modules = _get_project_modules(script_dir, main_module_name)
    importlib_ = importlib
    for mod_name in project_modules:
        mod = sys.modules.get(mod_name)
        if mod is not None:
            importlib_.reload(mod)

    # Get fresh main module
    new_main = sys.modules.get(main_module_name)
    if new_main is None:
        print(f"[hotreload] Module '{main_module_name}' lost after reload!")
        return False

    # Build old_class -> new_class mapping
    class_map = _build_class_map(old_modules)

    # Restore non-primitive runtime state that still exists in the new module
    # Primitives (int, float, str, bool, None, bytes) keep new code values
    for key, value in old_globals.items():
        if hasattr(new_main, key):
            if not isinstance(value, (
                int, float, str, bool, bytes, bytearray, type(None)
            )):
                setattr(new_main, key, value)

    # Auto-reclass instances to use reloaded class definitions
    for value in old_globals.values():
        if isinstance(value, (int, float, str, bool, bytes, bytearray, type(None))):
            continue
        _reclass_value(value, class_map)

    # Call optional on_reload hook
    if hasattr(new_main, 'on_reload'):
        new_main.on_reload(new_main)

    print(f"[hotreload] {len(project_modules)} module(s) reloaded, "
          f"{len(class_map)} class(es) updated")
    return True
