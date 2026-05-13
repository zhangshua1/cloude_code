import pygame
import math
from enum import Enum

from config import (
    CELL_SIZE, GRID_WIDTH, GRID_HEIGHT, WINDOW_WIDTH, WINDOW_HEIGHT,
    INITIAL_FPS, SPEED_INCREMENT, SPEED_UP_EVERY, MAX_FPS, INFO_BAR_HEIGHT,
    BG_COLOR, GRID_COLOR, SNAKE_HEAD_COLOR, SNAKE_BODY_COLOR,
    FOOD_COLOR, FOOD_GLOW_COLOR, TEXT_COLOR, SCORE_COLOR, OVERLAY_COLOR,
    UP, DOWN, LEFT, RIGHT,
)
from snake import Snake
from food import Food
from utils import load_high_score, save_high_score


class State(Enum):
    READY = "ready"
    PLAYING = "playing"
    PAUSED = "paused"
    GAME_OVER = "game_over"


class Game:
    def __init__(self):
        pygame.init()
        self.screen = pygame.display.set_mode((WINDOW_WIDTH, WINDOW_HEIGHT))
        pygame.display.set_caption("贪吃蛇")
        self.clock = pygame.time.Clock()
        self.font_large = pygame.font.SysFont("microsoftyahei", 36, bold=True)
        self.font_medium = pygame.font.SysFont("microsoftyahei", 22)
        self.font_small = pygame.font.SysFont("microsoftyahei", 16)
        self.reset()

    def reset(self):
        self.snake = Snake()
        self.food = Food()
        self.food.spawn(self.snake.body)
        self.score = 0
        self.high_score = load_high_score()
        self.fps = INITIAL_FPS
        self.state = State.READY
        self.frame_counter = 0

    def current_speed(self):
        extra = self.score // SPEED_UP_EVERY * SPEED_INCREMENT
        return min(INITIAL_FPS + extra, MAX_FPS)

    def handle_events(self):
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                return False

            if event.type == pygame.KEYDOWN:
                if self.state == State.READY:
                    if event.key in (pygame.K_UP, pygame.K_DOWN, pygame.K_LEFT, pygame.K_RIGHT):
                        self.state = State.PLAYING
                        self._handle_direction(event.key)
                    elif event.key == pygame.K_SPACE:
                        self.state = State.PLAYING

                elif self.state == State.PLAYING:
                    self._handle_direction(event.key)
                    if event.key == pygame.K_SPACE:
                        self.state = State.PAUSED

                elif self.state == State.PAUSED:
                    if event.key == pygame.K_SPACE:
                        self.state = State.PLAYING

                elif self.state == State.GAME_OVER:
                    if event.key == pygame.K_r:
                        self.reset()

        return True

    def _handle_direction(self, key):
        if key == pygame.K_UP:
            self.snake.change_direction(UP)
        elif key == pygame.K_DOWN:
            self.snake.change_direction(DOWN)
        elif key == pygame.K_LEFT:
            self.snake.change_direction(LEFT)
        elif key == pygame.K_RIGHT:
            self.snake.change_direction(RIGHT)

    def update(self):
        if self.state != State.PLAYING:
            return

        self.snake.move()

        if self.snake.check_wall_collision() or self.snake.check_self_collision():
            self.state = State.GAME_OVER
            if save_high_score(self.score):
                self.high_score = self.score
            return

        if self.snake.head() == self.food.position:
            self.snake.grow()
            self.score += 1
            self.fps = self.current_speed()
            self.food.spawn(self.snake.body)

    def render(self):
        self.screen.fill(BG_COLOR)

        # 网格
        for x in range(0, WINDOW_WIDTH, CELL_SIZE):
            pygame.draw.line(self.screen, GRID_COLOR, (x, INFO_BAR_HEIGHT),
                             (x, WINDOW_HEIGHT))
        for y in range(INFO_BAR_HEIGHT, WINDOW_HEIGHT, CELL_SIZE):
            pygame.draw.line(self.screen, GRID_COLOR, (0, y),
                             (WINDOW_WIDTH, y))

        # 蛇
        for i, (sx, sy) in enumerate(self.snake.body):
            px = sx * CELL_SIZE
            py = sy * CELL_SIZE + INFO_BAR_HEIGHT
            color = SNAKE_HEAD_COLOR if i == 0 else SNAKE_BODY_COLOR
            radius = CELL_SIZE // 2 - 2
            center = (px + CELL_SIZE // 2, py + CELL_SIZE // 2)
            pygame.draw.circle(self.screen, color, center, radius)

        # 食物（带闪烁）
        pulse = (math.sin(pygame.time.get_ticks() * 0.005) + 1) / 2
        fx, fy = self.food.position
        fpx = fx * CELL_SIZE + CELL_SIZE // 2
        fpy = fy * CELL_SIZE + INFO_BAR_HEIGHT + CELL_SIZE // 2
        glow_radius = int(CELL_SIZE // 2 + pulse * 4)
        pygame.draw.circle(self.screen, FOOD_GLOW_COLOR, (fpx, fpy), glow_radius)
        pygame.draw.circle(self.screen, FOOD_COLOR, (fpx, fpy), CELL_SIZE // 2 - 2)

        # 信息栏
        self._draw_info_bar()

        # 叠加层
        if self.state == State.READY:
            self._draw_overlay("贪 吃 蛇", "方向键移动 · 空格暂停 · R 重来", "按任意方向键开始")
        elif self.state == State.PAUSED:
            self._draw_overlay("暂 停", "", "按空格键继续")
        elif self.state == State.GAME_OVER:
            self._draw_overlay("游 戏 结 束", f"得分: {self.score}  最高分: {self.high_score}", "按 R 键重新开始")

        pygame.display.flip()

    def _draw_info_bar(self):
        bar_rect = pygame.Rect(0, 0, WINDOW_WIDTH, INFO_BAR_HEIGHT)
        pygame.draw.rect(self.screen, (15, 22, 15), bar_rect)
        pygame.draw.line(self.screen, (40, 60, 40), (0, INFO_BAR_HEIGHT),
                         (WINDOW_WIDTH, INFO_BAR_HEIGHT), 2)

        score_text = self.font_medium.render(f"分数: {self.score}", True, SCORE_COLOR)
        self.screen.blit(score_text, (16, 8))

        high_text = self.font_small.render(f"最高分: {self.high_score}", True, TEXT_COLOR)
        self.screen.blit(high_text, (16, 34))

        speed_text = self.font_small.render(f"速度: {self.fps}", True, TEXT_COLOR)
        self.screen.blit(speed_text, (WINDOW_WIDTH - 100, 8))

        state_map = {State.READY: "就绪", State.PLAYING: "运行中",
                     State.PAUSED: "暂停", State.GAME_OVER: "结束"}
        state_text = self.font_small.render(state_map.get(self.state, ""), True, TEXT_COLOR)
        self.screen.blit(state_text, (WINDOW_WIDTH - 100, 34))

    def _draw_overlay(self, title, subtitle, hint):
        overlay = pygame.Surface((WINDOW_WIDTH, WINDOW_HEIGHT), pygame.SRCALPHA)
        overlay.fill((0, 0, 0, 160))
        self.screen.blit(overlay, (0, 0))

        title_surf = self.font_large.render(title, True, SCORE_COLOR)
        tx = WINDOW_WIDTH // 2 - title_surf.get_width() // 2
        self.screen.blit(title_surf, (tx, WINDOW_HEIGHT // 2 - 60))

        if subtitle:
            sub_surf = self.font_small.render(subtitle, True, TEXT_COLOR)
            sx = WINDOW_WIDTH // 2 - sub_surf.get_width() // 2
            self.screen.blit(sub_surf, (sx, WINDOW_HEIGHT // 2 - 10))

        hint_surf = self.font_medium.render(hint, True, TEXT_COLOR)
        hx = WINDOW_WIDTH // 2 - hint_surf.get_width() // 2
        self.screen.blit(hint_surf, (hx, WINDOW_HEIGHT // 2 + 30))

    def run(self):
        running = True
        while running:
            running = self.handle_events()
            self.update()
            self.render()
            self.clock.tick(self.fps)

        pygame.quit()
