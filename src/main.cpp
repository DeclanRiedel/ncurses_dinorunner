#include <ncurses.h>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>
#include <cmath>
#include <array>

// Constants
const double PI = 3.14159265358979323846;
const int GROUND_OFFSET = 4;
const char DINO = 'D';
const char OBSTACLE = '#';
const char STONE = 'O';
const char STAR = '*';
const char BUSH = '"';
const char BUSH_ALT = '*';  // Add this line for the alternative bush shape
const int MAX_OBSTACLE_HEIGHT = 3;
const double JUMP_HEIGHT = 5.0;
const double JUMP_DURATION = 16.0;  // Reduced from 20.0 to make the jump faster
const int MIN_OBSTACLE_DISTANCE = 11;  // Reduced from 32 (about 2/3 reduction)
const int MAX_OBSTACLE_DISTANCE = 21;  // Reduced from 64 (about 2/3 reduction)
const int GAME_SPEED = 75;  // Increased from 50 (1/3 slower)

// Cactus shapes
const std::array<std::vector<std::string>, 3> CACTUS_SHAPES = {{
    {"#", "#", "#"},
    {"#", "##", "#"},
    {" #", "##", " #"}
}};

// GameObject struct
struct GameObject {
    int x, y;
    std::vector<std::string> shape;
    int height;
    int width;
    bool isCactus;
    int color;
};

// Function prototypes
bool checkCollision(const GameObject& a, const GameObject& b);
void setup();
void createObstacle();
void initializeStars();
void createBush();
void updateGame();
void draw();

// Global variables
int WIDTH, HEIGHT, GROUND;
std::vector<GameObject> obstacles, stars, bushes;
GameObject dino;
int score = 0;
bool gameOver = false;
double jumpProgress = 0.0;
bool isJumping = false;
int nextObstacleDistance = 0;

// Collision detection
bool checkCollision(const GameObject& a, const GameObject& b) {
    for (int ay = 0; ay < a.height; ay++) {
        for (int ax = 0; ax < a.shape[ay].length(); ax++) {
            if (a.shape[ay][ax] != ' ') {
                for (int by = 0; by < b.height; by++) {
                    for (int bx = 0; bx < b.shape[by].length(); bx++) {
                        if (b.shape[by][bx] != ' ' && a.x + ax == b.x + bx && a.y + ay == b.y + by) {
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

// Setup ncurses and game environment
void setup() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    timeout(GAME_SPEED);
    srand(time(0));
    
    getmaxyx(stdscr, HEIGHT, WIDTH);
    GROUND = HEIGHT - GROUND_OFFSET;

    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);  // Yellow will represent brown
    init_pair(3, COLOR_GREEN, COLOR_BLACK);   // Green for bushes
}

// Create a new obstacle
void createObstacle() {
    if (nextObstacleDistance <= 0) {
        bool isCactus = (rand() % 2 == 0);
        std::vector<std::string> shape;
        int height, width;
        
        if (isCactus) {
            shape = CACTUS_SHAPES[rand() % CACTUS_SHAPES.size()];
            height = shape.size();
            width = shape[0].length();
        } else {
            shape = {std::string(1, STONE)};
            height = 1;
            width = 1;
        }
        
        obstacles.push_back({WIDTH - 1, GROUND - height + 1, shape, height, width, isCactus, 0});
        nextObstacleDistance = rand() % (MAX_OBSTACLE_DISTANCE - MIN_OBSTACLE_DISTANCE + 1) + MIN_OBSTACLE_DISTANCE;
    } else {
        nextObstacleDistance--;
    }
}

// Initialize stars
void initializeStars() {
    const int NUM_STARS = WIDTH / 4;
    for (int i = 0; i < NUM_STARS; i++) {
        stars.push_back({rand() % WIDTH, rand() % (GROUND - 5), {std::string(1, STAR)}, 1, 1, false, 0});
    }
}

// Create a new bush
void createBush() {
    if (rand() % 15 == 0) {
        char bushShape = (rand() % 2 == 0) ? BUSH : BUSH_ALT;
        int color = (rand() % 2 == 0) ? 2 : 3;  // 2 for brown (yellow), 3 for green
        int yOffset = (rand() % 2) * 2 + 1; // Either 1 or 3 rows below the ground
        bushes.push_back({WIDTH - 1, GROUND + yOffset, {std::string(1, bushShape)}, 1, 1, false, color});
    }
}

// Update game state
void updateGame() {
    // Move and remove off-screen obstacles
    for (auto& obj : obstacles) {
        obj.x -= 1;  // Changed from 2 to 1 to slow down obstacle movement
    }
    obstacles.erase(std::remove_if(obstacles.begin(), obstacles.end(),
        [](const GameObject& obj) { return obj.x + obj.width < 0; }), obstacles.end());

    // Move and remove off-screen bushes
    for (auto& bush : bushes) {
        bush.x -= 1;  // Changed from 2 to 1 to slow down bush movement
    }
    bushes.erase(std::remove_if(bushes.begin(), bushes.end(),
        [](const GameObject& bush) { return bush.x + bush.width < 0; }), bushes.end());

    // Update jump
    if (isJumping) {
        jumpProgress += 2.0;  // Increased from 1.5 to 2.0 to make the jump faster
        if (jumpProgress >= JUMP_DURATION) {
            isJumping = false;
            jumpProgress = 0.0;
            dino.y = GROUND;
        } else {
            double jumpOffset = sin(jumpProgress / JUMP_DURATION * PI) * JUMP_HEIGHT;
            dino.y = GROUND - static_cast<int>(jumpOffset);
        }
    }

    // Check for collisions
    for (const auto& obj : obstacles) {
        if (checkCollision(dino, obj)) {
            gameOver = true;
            return;
        }
    }

    score += 1;  // Changed from 2 to 1 to slow down score increase
}

// Draw game state
void draw() {
    clear();

    // Draw stars
    for (const auto& star : stars) {
        mvaddch(star.y, star.x, star.shape[0][0]);
    }

    // Draw ground
    for (int i = 0; i < WIDTH; i++) {
        mvaddch(GROUND, i, '-');
    }

    // Draw obstacles
    for (const auto& obj : obstacles) {
        attron(COLOR_PAIR(obj.isCactus ? 1 : 2));
        for (int i = 0; i < obj.height; i++) {
            mvprintw(GROUND - obj.height + 1 + i, obj.x, obj.shape[i].c_str());
        }
        attroff(COLOR_PAIR(obj.isCactus ? 1 : 2));
    }

    // Draw dino
    mvaddch(dino.y, dino.x, dino.shape[0][0]);

    // Draw bushes
    for (const auto& bush : bushes) {
        attron(COLOR_PAIR(bush.color));
        mvaddch(bush.y, bush.x, bush.shape[0][0]);
        attroff(COLOR_PAIR(bush.color));
    }

    // Draw score
    mvprintw(0, 0, "Score: %d", score);

    refresh();
}

// Add these constants at the top of your file
const std::vector<std::string> DEFEAT_BANNER = {
    "______      __           _     _ ",
    "|  _  \\    / _|         | |   | |",
    "| | | |___| |_ ___  __ _| |_  | |",
    "| | | / _ \\  _/ _ \\/ _` | __| | |",
    "| |/ /  __/ ||  __/ (_| | |_  |_|",
    "|___/ \\___|_| \\___|\\__,_|\\__| (_)",
    "                                 ",
    "                                 "
};

int main() {
    setup();
    dino = {10, GROUND, {std::string(1, DINO)}, 1, 1, false, 0};
    
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

    // Game over sequence
    clear();

    // Display defeat banner
    int startY = (HEIGHT - DEFEAT_BANNER.size()) / 2 - 2;
    int startX = (WIDTH - DEFEAT_BANNER[0].length()) / 2;
    for (size_t i = 0; i < DEFEAT_BANNER.size(); ++i) {
        mvprintw(startY + i, startX, DEFEAT_BANNER[i].c_str());
    }

    // Display final score
    mvprintw(startY + DEFEAT_BANNER.size() + 1, WIDTH / 2 - 7, "Final Score: %d", score);

    // Display quit prompt
    mvprintw(startY + DEFEAT_BANNER.size() + 3, WIDTH / 2 - 10, "Press 'q' to quit");

    refresh();

    // Wait for 'q' to quit
    int ch;
    while ((ch = getch()) != 'q' && ch != 'Q') {
        // Wait for the correct key
    }

    endwin();
    return 0;
}