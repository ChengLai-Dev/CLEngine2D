import clengine


class MenuScene:
    """Example menu scene with title text simulation."""

    def __init__(self):
        self.scene = clengine.Scene()
        self.title_tex = clengine.load_texture("assets/textures/checkerboard.png")
        if self.title_tex:
            self.title_sprite = self.scene.create_sprite(
                "title", self.title_tex,
                x=500.0, y=300.0, w=256.0, h=64.0
            )

    def on_update(self, dt):
        if clengine.is_key_pressed(clengine.KeyCode.Enter):
            print("Enter pressed - switching to game scene")
            return "game"
        return None

    def get_scene(self):
        return self.scene
