# CLEngine2D Python Sandbox Entry
#
# Hook into the engine lifecycle: on_init, on_update,
# on_render, on_shutdown.

import sys
import debugpy
debugpy.connect(("localhost", 5678))
debugpy.wait_for_client()

from CLEngine.Math import Vec2, Vec3
from CLEngine.Input import (
    KeyCode, InputAction, InputMappingContext, InputSystem
)
from CLEngine.SceneGraph import Scene, LoadTexture, SceneManager
from CLEngine.Renderer import SetClearColor, GetRenderer, GetGameCamera
import CLEngine


# -- Hot-reload trigger state --
_reload_ctx = None


def _setup_reload_trigger():
    global _reload_ctx
    if _reload_ctx is not None:
        InputSystem.GetInstance().RemoveContext(_reload_ctx)
    reload_action = InputAction()
    reload_action.OnTriggered(lambda value: CLEngine.ReloadScripts())
    _reload_ctx = InputMappingContext()
    _reload_ctx.MapKey(reload_action, KeyCode.F5, Vec2(1, 0))
    InputSystem.GetInstance().AddContext(_reload_ctx)


# -- Game state --
scene = None
player = None
move_speed = 5.0


def on_move(value):
    global player
    if player is None:
        return
    dt = CLEngine.GetDeltaTime()
    dx = value.x * move_speed * dt
    dy = value.y * move_speed * dt
    if dx != 0.0 or dy != 0.0:
        pos = player.GetPosition()
        player.SetPosition(Vec3(pos.x + dx, pos.y + dy, 0.0))


def on_init():
    global scene, player

    SetClearColor(0.1, 0.1, 0.15, 1.0)

    scene = Scene()

    player_tex = LoadTexture("assets/textures/checkerboard.png")
    player = scene.CreateSprite(
        "player", player_tex,
        x=0.0, y=0.0, w=2.0, h=2.0
    )

    tex = LoadTexture("assets/textures/checkerboard.png")
    for i in range(3):
        ex = (i - 1) * 2.5
        scene.CreateSprite(
            f"enemy_{i}", tex,
            x=ex, y=3.0, w=1.5, h=1.5
        )

    action = InputAction()
    action.OnTriggered(on_move)
    ctx = InputMappingContext()
    ctx.MapKey(action, KeyCode.W, Vec2(0.0, 1.0))
    ctx.MapKey(action, KeyCode.Up, Vec2(0.0, 1.0))
    ctx.MapKey(action, KeyCode.S, Vec2(0.0, -1.0))
    ctx.MapKey(action, KeyCode.Down, Vec2(0.0, -1.0))
    ctx.MapKey(action, KeyCode.D, Vec2(1.0, 0.0))
    ctx.MapKey(action, KeyCode.Right, Vec2(1.0, 0.0))
    ctx.MapKey(action, KeyCode.A, Vec2(-1.0, 0.0))
    ctx.MapKey(action, KeyCode.Left, Vec2(-1.0, 0.0))
    InputSystem.GetInstance().AddContext(ctx)

    SceneManager.GetInstance().PushScene(scene)

    _setup_reload_trigger()


def on_reload(new_module):
    _setup_reload_trigger()


def on_update(dt):
    if scene:
        scene.OnUpdate(dt)


def on_render():
    renderer = GetRenderer()
    camera = GetGameCamera()
    if renderer and camera:
        pass


def on_shutdown():
    global player, scene
    print("Python game objects cleaned up")
