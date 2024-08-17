#include <ncurses.h>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>
#include <cmath>

const double PI = 3.14159265358979323846;

int WIDTH, HEIGHT;
const int GROUND_OFFSET = 4;
int GROUND;
const char DINO = 'D';
const char OBSTACLE = '#';
const char STAR = '*';
const char BUSH = '"';
const int MAX_OBSTACLE_HEIGHT = 3;
const double JUMP_HEIGHT = 5.0;
const double JUMP_DURATION = 20.0;

struct GameObject {
    int x, y;
    char symbol;
    int height;
};

std::vector<GameObject> obstacles;
std::vector<GameObject> stars;
std::vector<GameObject> bushes;
GameObject dino;
int score = 0;
bool gameOver = false;
double jumpProgress = 0.0;
bool isJumping = false;
int nextObstacleDistance = 0;

void setup() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    timeout(100);
    srand(time(0));
    
    // Get screen dimensions
    getmaxyx(stdscr, HEIGHT, WIDTH);
    GROUND = HEIGHT - GROUND_OFFSET;

    // Enable color
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);
}

void createObstacle() {
    if (nextObstacleDistance <= 0) {
        int height = rand() % MAX_OBSTACLE_HEIGHT + 1;
        obstacles.push_back({WIDTH - 1, GROUND - height + 1, OBSTACLE, height});
        nextObstacleDistance = rand() % 17 + 16; // Random distance between 16 and 32
    } else {
        nextObstacleDistance--;
    }
}

void initializeStars() {
    const int NUM_STARS = WIDTH / 4; // Adjust number of stars based on screen width
    for (int i = 0; i < NUM_STARS; i++) {
        stars.push_back({rand() % WIDTH, rand() % (GROUND - 5), STAR});
    }
}

void createBush() {
    if (rand() % 15 == 0) {
        bushes.push_back({WIDTH - 1, GROUND + 1, BUSH});
    }
}

void updateGame() {
    // Move obstacles
    for (auto& obj : obstacles) {
        obj.x--;
    }
    obstacles.erase(std::remove_if(obstacles.begin(), obstacles.end(),
        [](const GameObject& obj) { return obj.x < 0; }), obstacles.end());

    // Move bushes
    for (auto& bush : bushes) {
        bush.x--;
    }
    bushes.erase(std::remove_if(bushes.begin(), bushes.end(),
        [](const GameObject& bush) { return bush.x < 0; }), bushes.end());

    // Update jump
    if (isJumping) {
        jumpProgress += 1.0;
        if (jumpProgress >= JUMP_DURATION) {
            isJumping = false;
            jumpProgress = 0.0;
            dino.y = GROUND;
        } else {
            double jumpOffset = sin(jumpProgress / JUMP_DURATION * PI) * JUMP_HEIGHT;
            dino.y = GROUND - static_cast<int>(jumpOffset);
        }
    }

    // Check collision
    for (const auto& obj : obstacles) {
        if (obj.x == dino.x && dino.y >= GROUND - obj.height + 1) {
            gameOver = true;
            return;
        }
    }

    score++;
}

void draw() {
    clear();

    // Draw stars
    for (const auto& star : stars) {
        mvaddch(star.y, star.x, star.symbol);
    }

    // Draw ground
    for (int i = 0; i < WIDTH; i++) {
        mvaddch(GROUND, i, '-');
    }

    // Draw bushes
    for (const auto& bush : bushes) {
        mvaddch(bush.y, bush.x, bush.symbol);
    }

    // Draw obstacles in red
    attron(COLOR_PAIR(1));
    for (const auto& obj : obstacles) {
        for (int i = 0; i < obj.height; i++) {
            mvaddch(GROUND - i, obj.x, obj.symbol);
        }
    }
    attroff(COLOR_PAIR(1));

    // Draw dino
    mvaddch(dino.y, dino.x, dino.symbol);

    // Draw score
    mvprintw(0, 0, "Score: %d", score);

    refresh();
}

int main() {
    setup();
    dino = {10, GROUND, DINO};
    
    initializeStars();

    while (!gameOver) {
        int ch = getch();
        if (ch == ' ' && !isJumping) {
            isJumping = true;
            jumpProgress = 0.0;
        }

        createObstacle();
        createBush();
        updateGame();
        draw();
    }

    clear();
    mvprintw(HEIGHT/2, WIDTH/2 - 5, "Game Over!");
    mvprintw(HEIGHT/2 + 1, WIDTH/2 - 7, "Final Score: %d", score);
    refresh();
    getch();

    endwin();
    return 0;
}