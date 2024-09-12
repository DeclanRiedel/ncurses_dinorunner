#include <ncurses.h>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>
#include <cmath>
#include <array>
#include <algorithm>

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
const double JUMP_HEIGHT = 6.0;  // Increased by 20% from 5.0
const double JUMP_DURATION = 17.6;  // Increased by 10% from 16.0
const int MIN_OBSTACLE_DISTANCE = 11;  // Reduced from 32 (about 2/3 reduction)
const int MAX_OBSTACLE_DISTANCE = 21;  // Reduced from 64 (about 2/3 reduction)
const int GAME_SPEED = 75;  // Increased from 50 (1/3 slower)

// Cactus shapes
const std::array<std::vector<std::string>, 3> CACTUS_SHAPES = {{
    {"#", "#", "#"},
    {"#", "##", "#"},
    {" #", "##", " #"}
}};

// Use enum class for better type safety
enum class ObjectType { Dino, Obstacle, Star, Bush };

// Simplify GameObject struct
struct GameObject {
    int x, y;
    std::vector<std::string> shape;
    ObjectType type;
    int color;
};

// Function prototypes
bool checkCollision(const GameObject& a, const GameObject& b);
void setup();
void createObject(ObjectType type);
void initializeStars();
void updateGame();
void draw();
void drawDino();

// Global variables
int WIDTH, HEIGHT, GROUND;
std::vector<GameObject> gameObjects;
GameObject dino;
int score = 0;
bool gameOver = false;
double jumpProgress = 0.0;
bool isJumping = false;
int nextObstacleDistance = 0;

// Collision detection
bool checkCollision(const GameObject& a, const GameObject& b) {
    for (int ay = 0; ay < a.shape.size(); ay++) {
        for (int ax = 0; ax < a.shape[ay].length(); ax++) {
            if (a.shape[ay][ax] != ' ') {
                for (int by = 0; by < b.shape.size(); by++) {
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

// Create a new object
void createObject(ObjectType type) {
    switch (type) {
        case ObjectType::Obstacle: {
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
            
            gameObjects.push_back({WIDTH - 1, GROUND - height + 1, shape, ObjectType::Obstacle, isCactus ? 1 : 2});
            break;
        }
        case ObjectType::Bush: {
            char bushShape = (rand() % 2 == 0) ? BUSH : BUSH_ALT;
            int color = (rand() % 2 == 0) ? 2 : 3;  // 2 for brown (yellow), 3 for green
            int yOffset = (rand() % 2) * 2 + 1; // Either 1 or 3 rows below the ground
            gameObjects.push_back({WIDTH - 1, GROUND + yOffset, {std::string(1, bushShape)}, ObjectType::Bush, color});
            break;
        }
        case ObjectType::Star: {
            gameObjects.push_back({rand() % WIDTH, rand() % (GROUND - 5), {std::string(1, STAR)}, ObjectType::Star, 0});
            break;
        }
    }
}

// Initialize stars
void initializeStars() {
    const int NUM_STARS = WIDTH / 4;
    for (int i = 0; i < NUM_STARS; i++) {
        createObject(ObjectType::Star);
    }
}

// Update game state
void updateGame() {
    // Move all objects except the dino
    for (auto& obj : gameObjects) {
        if (obj.type != ObjectType::Star && obj.type != ObjectType::Dino) {
            obj.x -= 1;
        }
    }

    // Remove off-screen objects, but not the dino
    gameObjects.erase(std::remove_if(gameObjects.begin(), gameObjects.end(),
        [](const GameObject& obj) { 
            return obj.type != ObjectType::Dino && obj.x + obj.shape[0].length() < 0; 
        }), gameObjects.end());

    // Update jump
    if (isJumping) {
        jumpProgress += 2.0;
        if (jumpProgress >= JUMP_DURATION) {
            isJumping = false;
            jumpProgress = 0.0;
        }
    }

    // Always update dino's position, whether jumping or not
    double jumpOffset = isJumping ? sin(jumpProgress / JUMP_DURATION * PI) * JUMP_HEIGHT : 0;
    dino.y = GROUND - static_cast<int>(jumpOffset);

    // Update dino's position in gameObjects
    for (auto& obj : gameObjects) {
        if (obj.type == ObjectType::Dino) {
            obj.y = dino.y;
            break;
        }
    }

    // Check for collisions
    for (const auto& obj : gameObjects) {
        if (obj.type == ObjectType::Obstacle && checkCollision(dino, obj)) {
            gameOver = true;
            return;
        }
    }

    score += 1;  // Changed from 2 to 1 to slow down score increase
}

// Draw game state
void draw() {
    clear();

    // Draw ground first
    for (int i = 0; i < WIDTH; i++) {
        mvaddch(GROUND, i, '-');
    }

    // Draw all game objects
    for (const auto& obj : gameObjects) {
        attron(COLOR_PAIR(obj.color));
        for (int i = 0; i < obj.shape.size(); i++) {
            for (int j = 0; j < obj.shape[i].length(); j++) {
                if (obj.shape[i][j] != ' ') {
                    mvaddch(obj.y + i, obj.x + j, obj.shape[i][j]);
                }
            }
        }
        attroff(COLOR_PAIR(obj.color));
    }

    // Draw dino separately to ensure it's always on top
    attron(COLOR_PAIR(dino.color));
    mvaddch(dino.y, dino.x, DINO);
    attroff(COLOR_PAIR(dino.color));

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

// Optionally, you can add a separate function to draw the dino
void drawDino() {
    attron(COLOR_PAIR(dino.color));
    mvprintw(dino.y, dino.x, "%s", dino.shape[0].c_str());
    attroff(COLOR_PAIR(dino.color));
}

int main() {
    setup();
    dino = {10, GROUND, {std::string(1, DINO)}, ObjectType::Dino, 0};
    gameObjects.push_back(dino);  // Add this line to include dino in gameObjects
    
    initializeStars();

    while (!gameOver) {
        int ch = getch();
        if (ch == ' ' && !isJumping) {
            isJumping = true;
            jumpProgress = 0.0;
        }

        if (nextObstacleDistance <= 0) {
            createObject(ObjectType::Obstacle);
            nextObstacleDistance = rand() % (MAX_OBSTACLE_DISTANCE - MIN_OBSTACLE_DISTANCE + 1) + MIN_OBSTACLE_DISTANCE;
        } else {
            nextObstacleDistance--;
        }

        if (rand() % 15 == 0) {
            createObject(ObjectType::Bush);
        }

        updateGame();
        draw();
    }

    // Game over sequence
    clear();

    // Display defeat banner
    int startY = (HEIGHT - DEFEAT_BANNER.size()) / 2 - 2;
    int startX = (WIDTH - DEFEAT_BANNER[0].length()) / 2;
    for (size_t i = 0; i < DEFEAT_BANNER.size(); ++i) {
        // Fix the mvprintw call
        mvprintw(startY + i, startX, "%s", DEFEAT_BANNER[i].c_str());
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