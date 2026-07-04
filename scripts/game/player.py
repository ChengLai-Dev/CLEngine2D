from CLEngine.Math import Vec2, Vec3
from CLEngine.Input import (
    KeyCode, InputAction, InputMappingContext, InputSystem
)
import CLEngine


class Player:
    """Example player controller wrapping a Sprite."""

    def __init__(self, sprite):
        self.sprite = sprite
        self.speed = 200.0
        self.health = 100
        self._setup_input()

    def _on_move(self, value):
        dt = CLEngine.GetDeltaTime()
        dx = value.x * self.speed * dt
        dy = value.y * self.speed * dt
        if dx != 0.0 or dy != 0.0:
            pos = self.sprite.GetPosition()
            self.sprite.SetPosition(Vec3(pos.x + dx, pos.y + dy, 0.0))

    def _setup_input(self):
        move_action = InputAction()
        move_action.OnTriggered(self._on_move)
        ctx = InputMappingContext()
        ctx.MapKey(move_action, KeyCode.W, Vec2(0.0, 1.0))
        ctx.MapKey(move_action, KeyCode.S, Vec2(0.0, -1.0))
        ctx.MapKey(move_action, KeyCode.D, Vec2(1.0, 0.0))
        ctx.MapKey(move_action, KeyCode.A, Vec2(-1.0, 0.0))
        InputSystem.GetInstance().AddContext(ctx)

    def take_damage(self, amount):
        self.health -= amount
        if self.health <= 0:
            self.die()

    def die(self):
        self.sprite.SetVisible(False)
