from CLEngine.SceneGraph import Scene, LoadTexture
from game.Player import Player
from game.Enemy import Enemy


class GameScene:
    """Example game scene with player and enemies."""

    def __init__(self):
        self.scene = Scene()
        self.player = None
        self.enemies = []

        # Create player
        tex = LoadTexture("assets/textures/checkerboard.png")
        if tex:
            sprite = self.scene.CreateSprite("player", tex, 0.0, 0.0, 64.0, 64.0)
            self.player = Player(sprite)

        # Create enemies
        for i in range(3):
            ex = (i - 1) * 150.0
            etex = LoadTexture("assets/textures/checkerboard.png")
            if etex:
                esprite = self.scene.CreateSprite(
                    f"enemy_{i}", etex,
                    x=ex, y=200.0, w=40.0, h=40.0
                )
                self.enemies.append(Enemy(esprite, 60.0 + i * 20.0))

    def on_update(self, dt):
        if self.player:
            player_pos = self.player.sprite.GetPosition()
            for enemy in self.enemies:
                enemy.update(dt, player_pos)

        self.scene.OnUpdate(dt)

    def get_scene(self):
        return self.scene
