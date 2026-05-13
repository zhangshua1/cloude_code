import random
from config import GRID_WIDTH, GRID_HEIGHT


class Food:
    def __init__(self):
        self.position = (0, 0)

    def spawn(self, snake_body):
        empty = [(x, y) for x in range(GRID_WIDTH) for y in range(GRID_HEIGHT)
                 if (x, y) not in snake_body]
        if empty:
            self.position = random.choice(empty)
