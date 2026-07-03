import clengine
from game.player import Player
from game.enemy import Enemy


class GameScene:
    """Example game scene with player and enemies."""

    def __init__(self):
        self.scene = clengine.Scene()
        self.player = None
        self.enemies = []

        # Create player
        tex = clengine.load_texture("assets/textures/checkerboard.png")
        if tex:
            sprite = self.scene.create_sprite("player", tex, 0.0, 0.0, 64.0, 64.0)
            self.player = Player(sprite)

        # Create enemies
        for i in range(3):
            ex = (i - 1) * 150.0
            etex = clengine.load_texture("assets/textures/checkerboard.png")
            if etex:
                esprite = self.scene.create_sprite(
                    f"enemy_{i}", etex,
                    x=ex, y=200.0, w=40.0, h=40.0
                )
                self.enemies.append(Enemy(esprite, 60.0 + i * 20.0))

    def on_update(self, dt):
        if self.player:
            self.player.update(dt)

        if self.player:
            player_pos = self.player.sprite.get_position()
            for enemy in self.enemies:
                enemy.update(dt, player_pos)

        self.scene.on_update(dt)

    def get_scene(self):
        return self.scene
