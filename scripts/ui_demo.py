import sys
import debugpy
debugpy.connect(("localhost", 5678))
debugpy.wait_for_client()

from CLEngine.SceneGraph import Scene, SceneManager, UISystem
from CLEngine.Renderer import SetClearColor, GetRenderer, GetGameCamera
from CLEngine.Input import InputAction, InputMappingContext, InputSystem, KeyCode
import CLEngine

_reload_ctx = None
scene = None

def _setup_reload_trigger():
    global _reload_ctx
    if _reload_ctx is not None:
        InputSystem.GetInstance().RemoveContext(_reload_ctx)
    action = InputAction()
    action.OnTriggered(lambda value: CLEngine.ReloadScripts())
    _reload_ctx = InputMappingContext()
    _reload_ctx.MapKey(action, KeyCode.F5, Vec2(1, 0))
    InputSystem.GetInstance().AddContext(_reload_ctx)

def on_init():
    global scene
    SetClearColor(0.2, 0.2, 0.3, 1.0)
    scene = Scene()
    SceneManager.GetInstance().PushScene(scene)
    UISystem.GetInstance().AddUI("assets/ui/UIDemo.cui", 0)
    _setup_reload_trigger()

def on_update(dt):
    if scene:
        scene.OnUpdate(dt)
        UISystem.GetInstance().ProcessEvents()

def on_render():
    renderer = GetRenderer()
    camera = GetGameCamera()
    if renderer and camera:
        pass

def on_shutdown():
    global scene
    print("UI demo cleaned up")
