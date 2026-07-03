# CLEngine2D Python Sandbox Entry
#
# Hook into the engine lifecycle: on_init, on_update,
# on_render, on_shutdown.

import clengine


# -- Game state --
scene = None
player = None
enemies = []
move_speed = 5.0
_input_action = None


def on_init():
    global scene, player, _input_action

    clengine.set_clear_color(0.1, 0.1, 0.15, 1.0)

    scene = clengine.Scene()

    player_tex = clengine.load_texture("assets/textures/checkerboard.png")
    player = scene.create_sprite(
        "player", player_tex,
        x=0.0, y=0.0, w=2.0, h=2.0
    )

    tex = clengine.load_texture("assets/textures/checkerboard.png")
    for i in range(3):
        ex = (i - 1) * 2.5
        scene.create_sprite(
            f"enemy_{i}", tex,
            x=ex, y=3.0, w=1.5, h=1.5
        )

    _input_action = clengine.InputAction()
    ctx = clengine.InputMappingContext()
    ctx.map_key(_input_action, clengine.KeyCode.W, clengine.Vec2(0.0, 1.0))
    ctx.map_key(_input_action, clengine.KeyCode.Up, clengine.Vec2(0.0, 1.0))
    ctx.map_key(_input_action, clengine.KeyCode.S, clengine.Vec2(0.0, -1.0))
    ctx.map_key(_input_action, clengine.KeyCode.Down, clengine.Vec2(0.0, -1.0))
    ctx.map_key(_input_action, clengine.KeyCode.D, clengine.Vec2(1.0, 0.0))
    ctx.map_key(_input_action, clengine.KeyCode.Right, clengine.Vec2(1.0, 0.0))
    ctx.map_key(_input_action, clengine.KeyCode.A, clengine.Vec2(-1.0, 0.0))
    ctx.map_key(_input_action, clengine.KeyCode.Left, clengine.Vec2(-1.0, 0.0))
    clengine.InputSystem.get_instance().add_context(ctx)

    clengine.SceneManager.get_instance().push_scene(scene)


def on_update(dt):
    global player

    if player is None:
        return

    value = _input_action.get_value()
    dx = value.x * move_speed * dt
    dy = value.y * move_speed * dt

    if dx != 0.0 or dy != 0.0:
        pos = player.get_position()
        player.set_position(clengine.Vec3(pos.x + dx, pos.y + dy, 0.0))

    if scene:
        scene.on_update(dt)


def on_render():
    renderer = clengine.get_renderer()
    camera = clengine.get_game_camera()
    if renderer and camera:
        pass


def on_shutdown():
    global player, scene, enemies
    print("Python game objects cleaned up")
