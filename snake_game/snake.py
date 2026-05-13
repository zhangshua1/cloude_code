from config import GRID_WIDTH, GRID_HEIGHT, UP, DOWN, LEFT, RIGHT


class Snake:
    def __init__(self):
        cx, cy = GRID_WIDTH // 2, GRID_HEIGHT // 2
        self.body = [(cx, cy), (cx - 1, cy), (cx - 2, cy)]
        self.direction = RIGHT
        self._grow = False

    def head(self):
        return self.body[0]

    def change_direction(self, new_dir):
        # 禁止反向
        opposite = (-self.direction[0], -self.direction[1])
        if new_dir != opposite:
            self.direction = new_dir

    def grow(self):
        self._grow = True

    def move(self):
        hx, hy = self.body[0]
        dx, dy = self.direction
        new_head = (hx + dx, hy + dy)
        self.body.insert(0, new_head)
        if self._grow:
            self._grow = False
        else:
            self.body.pop()

    def check_wall_collision(self):
        hx, hy = self.head()
        return hx < 0 or hx >= GRID_WIDTH or hy < 0 or hy >= GRID_HEIGHT

    def check_self_collision(self):
        return self.head() in self.body[1:]
