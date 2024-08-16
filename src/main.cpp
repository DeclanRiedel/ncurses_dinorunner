#include <ncurses.h>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <random>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Game constants
const int SCREEN_WIDTH = 80;
const int SCREEN_HEIGHT = 24;
const int DINO_POS_X = 10;
const int GROUND_Y = SCREEN_HEIGHT - 4;
const int JUMP_HEIGHT = 8;  // Increased from 5 to 8
const int JUMP_DURATION = 16;  // New constant for jump duration

// Game objects
struct Dino {
    int y = GROUND_Y;
    int jumpCounter = 0;
    bool isJumping = false;
};

struct Obstacle {
    int x;
    int y;
};

// Function prototypes
void setup();
void draw(const Dino& dino, const std::vector<Obstacle>& obstacles, int score);
void update(Dino& dino, std::vector<Obstacle>& obstacles, int& score);

int main() {
    setup();

    Dino dino;
    std::vector<Obstacle> obstacles;
    int score = 0;

    while (true) {
        draw(dino, obstacles, score);
        update(dino, obstacles, score);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));  // Changed from 50ms to 100ms
    }

    endwin();
    return 0;
}

void setup() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    timeout(0);
}

void draw(const Dino& dino, const std::vector<Obstacle>& obstacles, int score) {
    clear();

    // Draw ground
    mvhline(GROUND_Y + 1, 0, '-', SCREEN_WIDTH);

    // Draw dino
    mvaddch(dino.y, DINO_POS_X, 'D');

    // Draw obstacles
    for (const auto& obs : obstacles) {
        mvaddch(obs.y, obs.x, '#');
    }

    // Draw score
    mvprintw(0, 0, "Score: %d", score);

    refresh();
}

void update(Dino& dino, std::vector<Obstacle>& obstacles, int& score) {
    // Handle input
    int ch = getch();
    if (ch == ' ' && !dino.isJumping) {
        dino.isJumping = true;
        dino.jumpCounter = 0;
    }

    // Update dino
    if (dino.isJumping) {
        if (dino.jumpCounter < JUMP_DURATION / 2) {
            dino.y = GROUND_Y - JUMP_HEIGHT * sin(M_PI * dino.jumpCounter / JUMP_DURATION);
        } else {
            dino.y = GROUND_Y - JUMP_HEIGHT * sin(M_PI * (JUMP_DURATION - dino.jumpCounter) / JUMP_DURATION);
        }
        dino.jumpCounter++;
        if (dino.jumpCounter >= JUMP_DURATION) {
            dino.isJumping = false;
            dino.y = GROUND_Y;
        }
    } else {
        dino.y = GROUND_Y;
    }

    // Update obstacles
    for (auto& obs : obstacles) {
        obs.x--;
    }
    obstacles.erase(std::remove_if(obstacles.begin(), obstacles.end(),
        [](const Obstacle& o) { return o.x < 0; }), obstacles.end());

    // Spawn new obstacles
    static std::default_random_engine rng(std::random_device{}());
    static std::uniform_int_distribution<int> dist(50, 150);
    if (obstacles.empty() || obstacles.back().x < SCREEN_WIDTH - dist(rng)) {
        obstacles.push_back({SCREEN_WIDTH, GROUND_Y});
    }

    // Check collisions
    for (const auto& obs : obstacles) {
        if (obs.x == DINO_POS_X && dino.y == GROUND_Y) {
            // Game over
            mvprintw(SCREEN_HEIGHT / 2, SCREEN_WIDTH / 2 - 5, "GAME OVER");
            refresh();
            getch();
            endwin();
            exit(0);
        }
    }
    // Update score
    score++;
}