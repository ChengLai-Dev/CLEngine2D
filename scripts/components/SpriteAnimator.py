class SpriteAnimator:
    """Example sprite animation using texture offset cycling."""

    def __init__(self, sprite, frame_count=4, fps=10.0):
        self.sprite = sprite
        self.frame_count = frame_count
        self.fps = fps
        self.time = 0.0
        self.current_frame = 0
        self.playing = True

    def update(self, dt):
        if not self.playing:
            return

        self.time += dt
        frame_duration = 1.0 / self.fps

        if self.time >= frame_duration:
            self.time -= frame_duration
            self.current_frame = (self.current_frame + 1) % self.frame_count
            frame_width = 1.0 / self.frame_count
            self.sprite.SetTexOffset(frame_width * self.current_frame, 0.0)
            self.sprite.SetTexScale(frame_width, 1.0)

    def play(self):
        self.playing = True

    def stop(self):
        self.playing = False
