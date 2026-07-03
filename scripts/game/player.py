import clengine


class Player:
    """Example player controller wrapping a Sprite."""

    def __init__(self, sprite):
        self.sprite = sprite
        self.speed = 200.0
        self.health = 100
        self._setup_input()

    def _setup_input(self):
        self._input_action = clengine.InputAction()
        ctx = clengine.InputMappingContext()
        ctx.map_key(self._input_action, clengine.KeyCode.W, clengine.Vec2(0.0, 1.0))
        ctx.map_key(self._input_action, clengine.KeyCode.S, clengine.Vec2(0.0, -1.0))
        ctx.map_key(self._input_action, clengine.KeyCode.D, clengine.Vec2(1.0, 0.0))
        ctx.map_key(self._input_action, clengine.KeyCode.A, clengine.Vec2(-1.0, 0.0))
        clengine.InputSystem.get_instance().add_context(ctx)

    def update(self, dt):
        value = self._input_action.get_value()
        dx = value.x * self.speed * dt
        dy = value.y * self.speed * dt

        if dx != 0.0 or dy != 0.0:
            pos = self.sprite.get_position()
            self.sprite.set_position(
                clengine.Vec3(pos.x + dx, pos.y + dy, 0.0)
            )

    def take_damage(self, amount):
        self.health -= amount
        if self.health <= 0:
            self.die()

    def die(self):
        self.sprite.set_visible(False)
