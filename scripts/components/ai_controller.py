import clengine
import math


class AIController:
    """Example AI controller for patrolling enemies."""

    def __init__(self, sprite, speed=60.0, patrol_range=100.0):
        self.sprite = sprite
        self.speed = speed
        self.patrol_range = patrol_range
        self.start_x = sprite.get_position().x
        self.direction = 1.0

    def update(self, dt):
        pos = self.sprite.get_position()
        new_x = pos.x + self.direction * self.speed * dt

        if abs(new_x - self.start_x) > self.patrol_range:
            self.direction *= -1.0
            new_x = pos.x

        self.sprite.set_position(
            clengine.Vec3(new_x, pos.y, pos.z)
        )
