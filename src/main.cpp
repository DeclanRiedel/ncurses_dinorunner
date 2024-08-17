#include <ncurses.h>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <random>
#include <cmath>
#include <array>
#include <deque>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Game constants
const int DINO_POS_X = 10;
const int GROUND_Y = 24 - 1;  // Bottom of the screen
const int ROAD_HEIGHT = 7;  // Increased road height
const int ROAD_TOP_Y = GROUND_Y - ROAD_HEIGHT + 1;
const int DINO_POS_Y = GROUND_Y - ROAD_HEIGHT / 2;  // Middle of the road
const char BUSH_CHAR = '*';
const char STAR_CHAR = '.';
const char HOUSE_CHAR = 'H';
const int NUM_STARS = 20;
const int NUM_HOUSES = 3;
const int JUMP_HEIGHT = 8;  // Reduced from 10 to 8
const int JUMP_DURATION = 16;  // Keep this the same
const int MIN_OBSTACLE_DISTANCE = 64;  // Doubled from 32
const int MAX_OBSTACLE_DISTANCE = 256; // Doubled from 128
const int ROAD_TOP = GROUND_Y - ROAD_HEIGHT + 1;
const int ROAD_BOTTOM = GROUND_Y - 1;
const double GAME_SPEED = 0.25; // 1/4 of the original speed
const double OBSTACLE_SPEED = GAME_SPEED * 2.90; // Double the previous speed (1.45 * 2)
const double BUSH_SPEED = GAME_SPEED; // Now moves at the same speed as houses
const int BUSH_SPACING = 5;  // Space between bushes

// Tetris shape definitions
const std::vector<std::vector<std::string>> TETRIS_SHAPES = {
    {"##", "##"},  // Square
    {"#", "#", "#", "#"},  // I
    {"#", "##", "#"},  // T
    {"##", " #", " #"},  // L
    {" #", " #", "##"},  // J
    {"##", "# ", "# "},  // S
    {"##", " #", " #"},  // Z
    {"###", "###", "###"},  // 3x3 block
    {"######", "######"}    // 2x6 block
};

// Game objects
struct Dino {
    int y = DINO_POS_Y;
    int jumpCounter = 0;
    bool isJumping = false;
    bool canDoubleJump = true;
};

struct Obstacle {
    double x;  // Changed from int to double
    int y;
    std::vector<std::string> shape;
};

// New struct for houses
struct House {
    double x;
    int y;
    int width;
    int height;
};

struct Bush {
    double x;
};

// New global variables
std::vector<std::pair<int, int>> stars;
std::deque<House> houses;
std::deque<Bush> bushes;

// Function prototypes
void setup();
void draw(const Dino& dino, const std::vector<Obstacle>& obstacles, int score, int screenWidth, int screenHeight);
void update(Dino& dino, std::vector<Obstacle>& obstacles, int& score, int screenWidth, int screenHeight);
void getScreenDimensions(int& width, int& height);
void drawLandscape(int screenWidth, int screenHeight);
char getRandomLandscapeChar();
void drawObstacle(const Obstacle& obstacle);
void drawBorder(int width, int height);
void drawColoredChar(int y, int x, char ch, int color);
void drawRoad(int screenWidth);
void drawBushes(int screenWidth);
char getRandomBackgroundChar();
void drawHouse(int x, int y, int width, int height);
void initializeStarsAndHouses(int screenWidth, int screenHeight);
void drawStarsAndHouses(int screenWidth);
void updateHouses(int screenWidth);
void initializeBushes(int screenWidth);
void updateBushes(int screenWidth);

void drawBorder(int width, int height) {
    // Top border
    mvaddch(0, 0, '+');
    mvhline(0, 1, '-', width - 2);
    mvaddch(0, width - 1, '+');

    // Side borders
    for (int i = 1; i < height - 1; ++i) {
        mvaddch(i, 0, '|');
        mvaddch(i, width - 1, '|');
    }

    // Bottom border
    mvaddch(height - 1, 0, '+');
    mvhline(height - 1, 1, '-', width - 2);
    mvaddch(height - 1, width - 1, '+');
}

void drawColoredChar(int y, int x, char ch, int color) {
    attron(COLOR_PAIR(color));
    mvaddch(y, x, ch);
    attroff(COLOR_PAIR(color));
}

void initializeStarsAndHouses(int screenWidth, int screenHeight) {
    // Initialize stars
    for (int i = 0; i < NUM_STARS; ++i) {
        int x = rand() % (screenWidth - 2) + 1;
        int y = rand() % (ROAD_TOP_Y - 1) + 1;
        stars.emplace_back(x, y);
    }

    // Initialize houses
    for (int i = 0; i < NUM_HOUSES; ++i) {
        double x = screenWidth + i * (screenWidth / NUM_HOUSES);
        int y = ROAD_TOP_Y - 1;
        int width = 5 + rand() % 3;  // Random width between 5 and 7
        int height = 3 + rand() % 2; // Random height between 3 and 4
        houses.push_back({x, y, width, height});
    }
}

void drawStarsAndHouses(int screenWidth) {
    // Draw stars
    attron(COLOR_PAIR(5));  // Star color
    for (const auto& star : stars) {
        mvaddch(star.second, star.first, STAR_CHAR);
    }
    attroff(COLOR_PAIR(5));

    // Draw houses
    attron(COLOR_PAIR(6));  // House color
    for (const auto& house : houses) {
        drawHouse(static_cast<int>(house.x), house.y - house.height, house.width, house.height);
    }
    attroff(COLOR_PAIR(6));
}

void drawHouse(int x, int y, int width, int height) {
    // Draw base
    for (int i = 0; i < height; ++i) {
        mvhline(y + i, x, '#', width);
    }
    
    // Draw roof
    for (int i = 0; i < width / 2 + 1; ++i) {
        mvaddch(y - i - 1, x + width / 2 - i, '/');
        mvaddch(y - i - 1, x + width / 2 + i, '\\');
    }
    
    // Draw door
    mvvline(y + height - 2, x + width / 3, '|', 2);
    
    // Draw window
    mvaddch(y + 1, x + 2 * width / 3, '+');
}

void updateHouses(int screenWidth) {
    for (auto& house : houses) {
        house.x -= GAME_SPEED;
    }

    // If the first house is off-screen, move it to the back
    if (houses.front().x + houses.front().width < 0) {
        House movedHouse = houses.front();
        houses.pop_front();
        movedHouse.x = screenWidth;
        movedHouse.width = 5 + rand() % 3;  // Random width between 5 and 7
        movedHouse.height = 3 + rand() % 2; // Random height between 3 and 4
        houses.push_back(movedHouse);
    }
}

void initializeBushes(int screenWidth) {
    for (int x = 2; x < screenWidth - 2; x += BUSH_SPACING) {
        if (rand() % 3 == 0) {  // 33% chance to place a bush
            bushes.push_back({static_cast<double>(x)});
        }
    }
}

void updateBushes(int screenWidth) {
    for (auto& bush : bushes) {
        bush.x -= BUSH_SPEED;  // Now moves at the same speed as houses
    }

    // If the first bush is off-screen, move it to the back
    if (!bushes.empty() && bushes.front().x < 0) {
        Bush movedBush = bushes.front();
        bushes.pop_front();
        movedBush.x = screenWidth - 1;
        bushes.push_back(movedBush);
    }

    // Add new bushes if there's space at the end
    while (bushes.empty() || bushes.back().x < screenWidth - BUSH_SPACING) {
        double newX = bushes.empty() ? screenWidth - 1 : bushes.back().x + BUSH_SPACING;
        if (rand() % 3 == 0) {  // 33% chance to place a bush
            bushes.push_back({newX});
        } else {
            bushes.push_back({newX + BUSH_SPACING});  // Skip this position
        }
    }
}

void drawBushes(int screenWidth) {
    attron(COLOR_PAIR(4));  // Bush color
    for (const auto& bush : bushes) {
        if (bush.x >= 0 && bush.x < screenWidth) {
            mvaddch(GROUND_Y + 1, static_cast<int>(bush.x), BUSH_CHAR);
        }
    }
    attroff(COLOR_PAIR(4));
}

char getRandomBackgroundChar() {
    const std::string chars = " .,:;";
    return chars[rand() % chars.length()];
}

void drawObstacle(const Obstacle& obstacle) {
    for (size_t y = 0; y < obstacle.shape.size(); ++y) {
        for (size_t x = 0; x < obstacle.shape[y].length(); ++x) {
            if (obstacle.shape[y][x] == '#') {
                drawColoredChar(obstacle.y + y, static_cast<int>(obstacle.x) + x, '#', 2);
            }
        }
    }
}

int main() {
    setup();

    int screenWidth, screenHeight;
    getScreenDimensions(screenWidth, screenHeight);

    initializeStarsAndHouses(screenWidth, screenHeight);
    initializeBushes(screenWidth);

    Dino dino;
    std::vector<Obstacle> obstacles;
    int score = 0;

    while (true) {
        draw(dino, obstacles, score, screenWidth, screenHeight);
        update(dino, obstacles, score, screenWidth, screenHeight);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
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
    
    // Initialize colors
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);  // Dino color
    init_pair(2, COLOR_RED, COLOR_BLACK);    // Obstacle color
    init_pair(3, COLOR_YELLOW, COLOR_BLACK); // Score color
    init_pair(4, COLOR_GREEN, COLOR_BLACK);  // Bush color
    init_pair(5, COLOR_WHITE, COLOR_BLACK);   // Star color
    init_pair(6, COLOR_YELLOW, COLOR_BLACK);  // House color
    init_pair(7, COLOR_WHITE, COLOR_BLACK);  // House details color
}

void draw(const Dino& dino, const std::vector<Obstacle>& obstacles, int score, int screenWidth, int screenHeight) {
    clear();

    // Draw border
    drawBorder(screenWidth, screenHeight);

    // Draw background
    for (int y = 1; y < ROAD_TOP_Y; ++y) {
        for (int x = 1; x < screenWidth - 1; ++x) {
            mvaddch(y, x, getRandomBackgroundChar());
        }
    }

    // Draw road
    drawRoad(screenWidth);

    // Draw bushes
    drawBushes(screenWidth);

    // Draw dino
    drawColoredChar(dino.y, DINO_POS_X, 'D', 1);

    // Draw obstacles
    for (const auto& obs : obstacles) {
        drawObstacle(obs);
    }

    // Draw stars and houses
    drawStarsAndHouses(screenWidth);

    // Draw score
    attron(COLOR_PAIR(3));
    mvprintw(1, 1, "Score: %d", score);
    attroff(COLOR_PAIR(3));

    refresh();
}

void drawRoad(int screenWidth) {
    // Draw top of the road
    mvhline(ROAD_TOP_Y, 1, '=', screenWidth - 2);
    
    // Draw bottom of the road
    mvhline(GROUND_Y, 1, '=', screenWidth - 2);
    
    // Draw sides of the road
    for (int y = ROAD_TOP_Y + 1; y < GROUND_Y; ++y) {
        mvaddch(y, 1, '|');
        mvaddch(y, screenWidth - 2, '|');
    }
}

void update(Dino& dino, std::vector<Obstacle>& obstacles, int& score, int screenWidth, int screenHeight) {
    // Handle input
    int ch = getch();
    if (ch == ' ') {
        if (!dino.isJumping) {
            dino.isJumping = true;
            dino.jumpCounter = 0;
        }
    }

    // Update dino
    if (dino.isJumping) {
        if (dino.jumpCounter < JUMP_DURATION / 2) {
            dino.y = DINO_POS_Y - JUMP_HEIGHT * sin(M_PI * dino.jumpCounter / JUMP_DURATION);
        } else {
            dino.y = DINO_POS_Y - JUMP_HEIGHT * sin(M_PI * (JUMP_DURATION - dino.jumpCounter) / JUMP_DURATION);
        }
        dino.jumpCounter++;
        if (dino.jumpCounter >= JUMP_DURATION) {
            dino.isJumping = false;
            dino.y = DINO_POS_Y;
        }
    } else {
        dino.y = DINO_POS_Y;
    }

    // Update obstacles
    for (auto& obs : obstacles) {
        obs.x -= OBSTACLE_SPEED;
    }
    obstacles.erase(std::remove_if(obstacles.begin(), obstacles.end(),
        [](const Obstacle& o) { return o.x + static_cast<int>(o.shape[0].length()) < 0; }), obstacles.end());

    // Spawn new obstacles
    static std::default_random_engine rng(std::random_device{}());
    static std::uniform_int_distribution<int> dist(MIN_OBSTACLE_DISTANCE, MAX_OBSTACLE_DISTANCE);
    static std::uniform_int_distribution<int> shape_dist(0, TETRIS_SHAPES.size() - 1);
    static std::uniform_int_distribution<int> y_dist(ROAD_TOP + 1, ROAD_BOTTOM - 1);

    if (obstacles.empty() || obstacles.back().x < screenWidth - dist(rng)) {
        int shape_index = shape_dist(rng);
        int shape_height = static_cast<int>(TETRIS_SHAPES[shape_index].size());
        int y_position = y_dist(rng) - shape_height + 1;
        
        y_position = std::max(y_position, ROAD_TOP);
        y_position = std::min(y_position, ROAD_BOTTOM - shape_height + 1);
        
        obstacles.push_back({static_cast<double>(screenWidth - 1), y_position, TETRIS_SHAPES[shape_index]});
    }

    // Check collisions
    for (const auto& obs : obstacles) {
        for (size_t y = 0; y < obs.shape.size(); ++y) {
            for (size_t x = 0; x < obs.shape[y].length(); ++x) {
                if (obs.shape[y][x] == '#' && 
                    static_cast<int>(obs.x) + x == DINO_POS_X && 
                    obs.y + y == dino.y) {
                    // Game over
                    attron(COLOR_PAIR(2));
                    mvprintw(screenHeight / 2, (screenWidth - 10) / 2, "GAME OVER");
                    attroff(COLOR_PAIR(2));
                    refresh();
                    getch();
                    endwin();
                    exit(0);
                }
            }
        }
    }

    // Update score
    static double scoreCounter = 0;
    scoreCounter += GAME_SPEED;
    if (scoreCounter >= 1) {
        score++;
        scoreCounter -= 1;
    }

    // Update houses and bushes
    updateHouses(screenWidth);
    updateBushes(screenWidth);
}

void getScreenDimensions(int& width, int& height) {
    getmaxyx(stdscr, height, width);
}