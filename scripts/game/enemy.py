import clengine
import math


class Enemy:
    """Simple enemy that moves toward the player."""

    def __init__(self, sprite, speed=80.0):
        self.sprite = sprite
        self.speed = speed
        self.active = True

    def update(self, dt, player_pos):
        if not self.active:
            return

        pos = self.sprite.get_position()

        dx = player_pos.x - pos.x
        dy = player_pos.y - pos.y
        dist = math.sqrt(dx * dx + dy * dy)

        if dist > 1.0:
            nx = dx / dist
            ny = dy / dist
            self.sprite.set_position(
                clengine.Vec3(
                    pos.x + nx * self.speed * dt,
                    pos.y + ny * self.speed * dt,
                    0.0
                )
            )
