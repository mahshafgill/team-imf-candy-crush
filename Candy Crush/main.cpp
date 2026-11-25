// =====================================================================
//  Team IMF - Candy Crush  (CS1002 Programming Fundamentals, Fall 2026)
//  Feature 1 (Mahshaf): game window and the input -> update -> draw game loop
// =====================================================================

#include <SFML/Graphics.hpp>
#include <optional>
#include <cstdlib>
#include <ctime>
#include <string>
#include <fstream>
using namespace std;

// ---------------- Window and board constants ----------------
const int WINDOW_W = 600;
const int WINDOW_H = 680;
const int HUD_HEIGHT = 80;          // strip at the top for score / moves / target
const int ROWS = 8;
const int COLS = 8;
const float CELL = 70.f;            // one cell is 70 x 70 pixels
const float BOARD_X = 20.f;         // top-left corner of the board
const float BOARD_Y = 100.f;
const float CANDY_RADIUS = 28.f;    // candy circle inside a 70 x 70 cell
const int POINTS = 10;              // points for every cleared candy

// ---------------- Game states ----------------
const int MENU = 0;
const int PLAYING = 1;
const int GAME_OVER = 2;

// ---------------- Keys we care about ----------------
const int KEY_NONE = 0;
const int KEY_ESCAPE = 1;
const int KEY_ENTER = 2;
const int KEY_UP = 3;
const int KEY_DOWN = 4;

// ---------------- Candy colours (index = number in the board) ----------------
const sf::Color candyColors[7] = {
    sf::Color(25, 25, 45),          // 0: empty
    sf::Color(235, 60, 70),         // 1: red
    sf::Color(255, 150, 0),         // 2: orange
    sf::Color(250, 215, 50),        // 3: yellow
    sf::Color(70, 200, 90),         // 4: green
    sf::Color(60, 130, 240),        // 5: blue
    sf::Color(170, 90, 255)         // 6: purple
};

// ---------------- Shared game data (used by all team members) ----------------
int board[ROWS][COLS];
int score = 0;
int movesLeft = 0;
int targetScore = 0;
int gameState = MENU;
int selRow = -1;                    // -1 means nothing is selected
int selCol = -1;
int frameCount = 0;                 // counts frames; used for animations and hint timer
bool playerWon = false;             // true = "YOU WIN", false = "GAME OVER"
sf::Font gameFont;                  // loaded once in main()
bool fontLoaded = false;

// ---------------- Input of the current frame ----------------
int clickX = 0;
int clickY = 0;
bool clicked = false;
int pressedKey = KEY_NONE;

// =================== IMAN: matches, clearing, falling, refilling (F4, F5) ===================

//Matching
int findMatches(bool marked[][8], int board[][8])
{
    int candies = 0;
    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            marked[r][c] = false;
        }
    }

    for (int r = 0; r < ROWS; r++)//ROWS
    {
        for (int c = 0; c < COLS - 2; c++)
        {
            if (board[r][c] == board[r][c + 1] && board[r][c] == board[r][c + 2] && board[r][c] != 0)
            {
                marked[r][c] = true;
                marked[r][c + 1] = true;
                marked[r][c + 2] = true;
            }
        }
    }
    for (int c = 0; c < COLS; c++)//COLS
    {
        for (int r = 0; r < ROWS - 2; r++)
        {
            if (board[r][c] == board[r + 1][c] && board[r][c] == board[r + 2][c] && board[r][c] != 0)
            {
                marked[r][c] = true;
                marked[r + 1][c] = true;
                marked[r + 2][c] = true;
            }
        }
    }
    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            if (marked[r][c] == true)
            {
                candies++;
            }
        }
    }
    return candies;
}

//Candy Clearing and Scoring
int clear_score(bool marked[][8], int board[][8])
{
    int obtained = 0;
    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            if (marked[r][c] == true)
            {
                board[r][c] = 0;
                obtained += POINTS;
            }
        }
    }
    return obtained;
}

//Falling
void dropcandies(int board[][8])
{
    for (int c = 0; c < COLS; c++)//Column Wise
    {
        int writeRow = 7;
        for (int r = 7; r >= 0; r--)
        {
            if (board[r][c])
            {
                board[writeRow][c] = board[r][c];
                writeRow--;
            }
        }
        while (writeRow >= 0)
        {
            board[writeRow][c] = 0;
            writeRow--;
        }
    }
}

//Refilling
void Refill(int board[][8])
{
    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            if (board[r][c] == 0)
            {
                board[r][c] = rand() % 6 + 1;
            }
        }
    }
}

//Chains
void chains(bool marked[][8], int board[][8])
{
    while (findMatches(marked, board))//when candy exists
    {
        score += clear_score(marked, board);
        dropcandies(board);
        Refill(board);
    }
}


// =================== FAIQA: level file, board drawing, clicks & swaps, save (F2, F3, F8) ===================

bool loadLevel(const string& filename)
{
    ifstream levelFile(filename);
    if (!levelFile.is_open())
    {
        return false;                       // missing file
    }

    int fileRows = 0;
    int fileCols = 0;
    int fileMoves = 0;
    int fileTarget = 0;

    if (!(levelFile >> fileRows >> fileCols >> fileMoves >> fileTarget))
    {
        levelFile.close();
        return false;                       // empty or broken header
    }

    if (fileRows != ROWS || fileCols != COLS || fileMoves <= 0 || fileTarget <= 0)
    {
        levelFile.close();
        return false;
    }

    int tempBoard[ROWS][COLS];

    for (int row = 0; row < ROWS; row++)
    {
        for (int col = 0; col < COLS; col++)
        {
            int cellValue = 0;
            if (!(levelFile >> cellValue) || cellValue < 0 || cellValue > 6)
            {
                levelFile.close();
                return false;
            }
            tempBoard[row][col] = cellValue;
        }
    }

    for (int row = 0; row < ROWS; row++)
    {
        for (int col = 0; col < COLS; col++)
        {
            board[row][col] = tempBoard[row][col];
        }
    }

    movesLeft = fileMoves;
    targetScore = fileTarget;
    score = 0;
    selRow = -1;
    selCol = -1;

    levelFile.close();
    return true;
}

void drawBoard(sf::RenderWindow& win)
{
    sf::RectangleShape cellBox({ CELL - 2.f, CELL - 2.f });
    sf::CircleShape candyCircle(CANDY_RADIUS);

    for (int row = 0; row < ROWS; row++)
    {
        for (int col = 0; col < COLS; col++)
        {
            float cellLeft = BOARD_X + col * CELL;
            float cellTop = BOARD_Y + row * CELL;

            // 1. dark square for every cell
            cellBox.setPosition({ cellLeft + 1.f, cellTop + 1.f });
            cellBox.setFillColor(sf::Color(35, 35, 65));
            win.draw(cellBox);

            int candyType = board[row][col];                    // 2. candy on top of it (0 = empty, so no candy)
            if (candyType >= 1 && candyType <= 6)
            {
                float centerX = cellLeft + CELL / 2.f;
                float centerY = cellTop + CELL / 2.f;

                candyCircle.setFillColor(candyColors[candyType]);
                candyCircle.setPosition({ centerX - CANDY_RADIUS, centerY - CANDY_RADIUS });
                win.draw(candyCircle);
            }
        }
    }
}

bool trySwap(int r1, int c1, int r2, int c2)
{
    bool neighbour = false;                      //Checking for Neighbours

    if (r1 == r2 && (c1 - c2 == 1 || c2 - c1 == 1))
    {
        neighbour = true;
    }
    if (c1 == c2 && (r1 - r2 == 1 || r2 - r1 == 1))
    {
        neighbour = true;
    }

    if (neighbour == false)
    {
        return false;
    }

    int temp = board[r1][c1];
    board[r1][c1] = board[r2][c2];
    board[r2][c2] = temp;

    bool marked[ROWS][COLS];
    int found = findMatches(marked, board);

    if (found == 0)
    {
        temp = board[r1][c1];
        board[r1][c1] = board[r2][c2];
        board[r2][c2] = temp;
        return false;
    }

    return true;
}


// =================== MAHSHAF: HUD, menu, hint & shuffle (F1, F6, F7, Bonus) ===================

int choose_menu = 0;                // Mahshaf - 0: New Game, 1: Continue, 2: Exit
string menu_ins = "";               // Mahshaf - error text if level or save file is missing

// Mahshaf - hint and shuffle variables (Bonus)
int wait_time = 0;                  // counts idle frames (300 = 5 sec)
bool hint_on = false;
int hr1 = -1, hc1 = -1, hr2 = -1, hc2 = -1;
int mix_time = 0;                   // shows "Shuffling..." for 60 frames

// Forward declarations
bool loadSavedGame();
void options(sf::RenderWindow& win);
bool fmove(int& r1, int& c1, int& r2, int& c2);
void shuffleboard();

// Temporary stub until Faiqa adds Feature 8 (savegame.txt)
bool loadSavedGame()
{
    return false;
}

// Mahshaf - prints text centered at (x, y)
void middletxt(sf::RenderWindow& win, const string& str, unsigned int size,
    sf::Color color, float x, float y)
{
    if (!fontLoaded)
        return;

    sf::Text text(gameFont, str, size);
    text.setFillColor(color);
    sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin({ bounds.position.x + bounds.size.x / 2.f,
                     bounds.position.y + bounds.size.y / 2.f });
    text.setPosition({ x, y });
    win.draw(text);
}

// Mahshaf - Feature 6: draws top HUD bar with score, moves, target, and progress bar
void hud(sf::RenderWindow& win)
{
    sf::RectangleShape hud({ (float)WINDOW_W, (float)HUD_HEIGHT });
    hud.setFillColor(sf::Color(20, 20, 45));
    win.draw(hud);

    float colw = WINDOW_W / 3.f;
    sf::Color labelcolor(170, 170, 210);

    middletxt(win, "SCORE", 14, labelcolor, colw * 0.5f, 20.f);
    middletxt(win, to_string(score), 30, sf::Color::White, colw * 0.5f, 46.f);

    sf::Color movescolor = sf::Color::White;
    if (movesLeft <= 5)
        movescolor = sf::Color(255, 90, 90);
    middletxt(win, "MOVES", 14, labelcolor, colw * 1.5f, 20.f);
    middletxt(win, to_string(movesLeft), 30, movescolor, colw * 1.5f, 46.f);

    middletxt(win, "TARGET", 14, labelcolor, colw * 2.5f, 20.f);
    middletxt(win, to_string(targetScore), 30, sf::Color(255, 215, 90), colw * 2.5f, 46.f);

    float progress = 0.f;
    if (targetScore > 0)
        progress = (float)score / targetScore;
    if (progress > 1.f)
        progress = 1.f;

    sf::RectangleShape barback({ (float)WINDOW_W, 6.f });
    barback.setPosition({ 0.f, HUD_HEIGHT - 6.f });
    barback.setFillColor(sf::Color(60, 40, 80));
    win.draw(barback);

    sf::RectangleShape barfill({ WINDOW_W * progress, 6.f });
    barfill.setPosition({ 0.f, HUD_HEIGHT - 6.f });
    barfill.setFillColor(sf::Color(255, 90, 160));
    win.draw(barfill);
}

// Mahshaf - Feature 7: draws YOU WIN / GAME OVER screen
void endscr(sf::RenderWindow& win)
{
    if (playerWon)
        middletxt(win, "YOU WIN!", 64, sf::Color(255, 215, 90), WINDOW_W / 2.f, 260.f);
    else
        middletxt(win, "GAME OVER", 64, sf::Color(255, 90, 90), WINDOW_W / 2.f, 260.f);

    middletxt(win, "Final score: " + to_string(score), 30, sf::Color::White, WINDOW_W / 2.f, 340.f);

    // blink "Press Enter" using frame count
    int pulse = (frameCount / 25) % 2;
    sf::Color fade = sf::Color::White;
    if (pulse == 1)
        fade = sf::Color(150, 150, 180);
    middletxt(win, "Press Enter", 22, fade, WINDOW_W / 2.f, 430.f);
}

// Mahshaf - Feature 6: checks if target score is reached or moves finished
void end()
{
    if (score >= targetScore)
    {
        playerWon = true;
        gameState = GAME_OVER;
    }
    else if (movesLeft <= 0)
    {
        playerWon = false;
        gameState = GAME_OVER;
    }
}

// Mahshaf - Feature 1: reads window close, mouse clicks, and keyboard keys
void input(sf::RenderWindow& win)
{
    clicked = false;
    pressedKey = KEY_NONE;

    while (const std::optional event = win.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
        {
            win.close();
        }
        else if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>())
        {
            if (mouse->button == sf::Mouse::Button::Left)
            {
                clickX = mouse->position.x;
                clickY = mouse->position.y;
                clicked = true;
            }
        }
        else if (const auto* key = event->getIf<sf::Event::KeyPressed>())
        {
            if (key->code == sf::Keyboard::Key::Escape)
                pressedKey = KEY_ESCAPE;
            else if (key->code == sf::Keyboard::Key::Enter)
                pressedKey = KEY_ENTER;
            else if (key->code == sf::Keyboard::Key::Up)
                pressedKey = KEY_UP;
            else if (key->code == sf::Keyboard::Key::Down)
                pressedKey = KEY_DOWN;
        }
    }
}

// Mahshaf - Bonus Backend: checks right and down swaps for every cell to find a valid move
bool fmove(int& r1, int& c1, int& r2, int& c2)
{
    bool m[8][8];
    int dr[2] = { 0, 1 };             // 0: right neighbour, 1: bottom neighbour
    int dc[2] = { 1, 0 };

    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            if (board[r][c] == 0)
                continue;

            for (int d = 0; d < 2; d++)
            {
                int nr = r + dr[d];
                int nc = c + dc[d];

                if (nr < ROWS && nc < COLS && board[nr][nc] != 0)
                {
                    int t = board[r][c];
                    board[r][c] = board[nr][nc];
                    board[nr][nc] = t;

                    int count = findMatches(m, board);

                    // swap back after checking
                    board[nr][nc] = board[r][c];
                    board[r][c] = t;

                    if (count > 0)
                    {
                        r1 = r; c1 = c;
                        r2 = nr; c2 = nc;
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

// Mahshaf - Bonus Backend: shuffles existing candies on the board if no valid moves exist
void shuffleboard()
{
    int r1, c1, r2, c2;
    if (fmove(r1, c1, r2, c2))
        return;

    bool m[8][8];
    bool ok = false;

    while (!ok)
    {
        // swap random pairs of cells across the board
        for (int r = 0; r < ROWS; r++)
        {
            for (int c = 0; c < COLS; c++)
            {
                int rr = rand() % ROWS;
                int cc = rand() % COLS;
                int temp = board[r][c];
                board[r][c] = board[rr][cc];
                board[rr][cc] = temp;
            }
        }

        if (findMatches(m, board) == 0 && fmove(r1, c1, r2, c2))
        {
            ok = true;
        }
    }

    selRow = -1;
    selCol = -1;
    wait_time = 0;
    hint_on = false;
    mix_time = 60;
}

// Mahshaf - Bonus: draws yellow hint boxes and "Shuffling..." alert
void hint_shuffle(sf::RenderWindow& win)
{
    if (hint_on && hr1 != -1 && (frameCount % 30 < 20))
    {
        sf::RectangleShape box({ CELL - 6.f, CELL - 6.f });
        box.setFillColor(sf::Color::Transparent);
        box.setOutlineThickness(4.f);
        box.setOutlineColor(sf::Color::Yellow);

        box.setPosition({ BOARD_X + hc1 * CELL + 3.f, BOARD_Y + hr1 * CELL + 3.f });
        win.draw(box);

        box.setPosition({ BOARD_X + hc2 * CELL + 3.f, BOARD_Y + hr2 * CELL + 3.f });
        win.draw(box);
    }

    if (mix_time > 0)
    {
        sf::RectangleShape msgbox({ 260.f, 60.f });
        msgbox.setOrigin({ 130.f, 30.f });
        msgbox.setPosition({ WINDOW_W / 2.f, BOARD_Y + 280.f });
        msgbox.setFillColor(sf::Color(20, 20, 45, 235));
        msgbox.setOutlineThickness(3.f);
        msgbox.setOutlineColor(sf::Color::Yellow);
        win.draw(msgbox);

        middletxt(win, "Shuffling...", 26, sf::Color::Yellow, WINDOW_W / 2.f, BOARD_Y + 278.f);
    }
}

// Mahshaf - Feature 7 & Bonus: updates current state and idle hint timer
void update(sf::RenderWindow& win)
{
    frameCount++;

    switch (gameState)
    {
    case MENU:
        if (pressedKey == KEY_UP)
        {
            choose_menu--;
            if (choose_menu < 0)
                choose_menu = 2;
        }
        else if (pressedKey == KEY_DOWN)
        {
            choose_menu++;
            if (choose_menu > 2)
                choose_menu = 0;
        }
        else if (pressedKey == KEY_ENTER)
        {
            options(win);
        }

        if (clicked)
        {
            for (int i = 0; i < 3; i++)
            {
                float y = 390.f + i * 65.f;
                if (clickX >= WINDOW_W / 2 - 120 && clickX <= WINDOW_W / 2 + 120 &&
                    clickY >= y - 24.f && clickY <= y + 24.f)
                {
                    choose_menu = i;
                    options(win);
                }
            }
        }
        break;

    case PLAYING:
        if (mix_time > 0)
            mix_time--;

        // Mahshaf - reset hint on click, otherwise trigger hint after 5 seconds (300 frames)
        if (clicked)
        {
            wait_time = 0;
            hint_on = false;
        }
        else
        {
            wait_time++;
            if (wait_time >= 300 && !hint_on)
            {
                if (fmove(hr1, hc1, hr2, hc2))
                    hint_on = true;
            }
        }

        // TEMPORARY test keys: Up = +50 score, Down = use one move
        if (pressedKey == KEY_UP)
            score += 50;
        if (pressedKey == KEY_DOWN)
            movesLeft--;

        if (pressedKey == KEY_ESCAPE)
            gameState = MENU;

        end();
        break;

    case GAME_OVER:
        if (pressedKey == KEY_ENTER)
            gameState = MENU;
        break;
    }
}

// Mahshaf - draws two-tone background
void bg(sf::RenderWindow& win, sf::Color top, sf::Color bottom)
{
    sf::RectangleShape bg({ (float)WINDOW_W, (float)WINDOW_H });
    bg.setFillColor(bottom);
    win.draw(bg);

    bg.setSize({ (float)WINDOW_W, 240.f });
    bg.setFillColor(top);
    win.draw(bg);
}

// Mahshaf - draws 6 bouncing candies on the Main Menu
void bounce(sf::RenderWindow& win, sf::CircleShape& candy)
{
    for (int i = 1; i <= 6; i++)
    {
        int step = (frameCount / 5 + i * 2) % 10;
        float offset = (step < 5) ? step * 3.f : (10 - step) * 3.f;
        candy.setFillColor(candyColors[i]);
        candy.setPosition({ 60.f + (i - 1) * 85.f, 290.f + offset });
        win.draw(candy);
    }
}

// Mahshaf - Feature 7: runs selected Main Menu option
void options(sf::RenderWindow& win)
{
    menu_ins = "";

    if (choose_menu == 0)           // New Game
    {
        if (loadLevel("level.txt"))
        {
            wait_time = 0;
            hint_on = false;
            shuffleboard();
            gameState = PLAYING;
        }
        else
        {
            menu_ins = "Error: level.txt not found!";
        }
    }
    else if (choose_menu == 1)      // Continue
    {
        if (loadSavedGame())
        {
            selRow = -1;
            selCol = -1;
            wait_time = 0;
            hint_on = false;
            shuffleboard();
            gameState = PLAYING;
        }
        else
        {
            menu_ins = "No saved game";
        }
    }
    else if (choose_menu == 2)      // Exit
    {
        win.close();
    }
}

// Mahshaf - Feature 7: draws Main Menu (New Game, Continue, Exit)
void menu(sf::RenderWindow& win)
{
    middletxt(win, "CANDY CRUSH", 54, sf::Color(255, 215, 90), WINDOW_W / 2.f, 145.f);
    middletxt(win, "Team IMF  |  CS1002", 18, sf::Color(180, 180, 220), WINDOW_W / 2.f, 195.f);

    const string options[3] = { "New Game", "Continue", "Exit" };
    float startY = 390.f;
    float gapY = 65.f;
    float btnW = 240.f;
    float btnH = 48.f;

    for (int i = 0; i < 3; i++)
    {
        float y = startY + i * gapY;
        bool isselected = (i == choose_menu);

        sf::RectangleShape btn({ btnW, btnH });
        btn.setOrigin({ btnW / 2.f, btnH / 2.f });
        btn.setPosition({ WINDOW_W / 2.f, y });

        if (isselected)
        {
            btn.setFillColor(sf::Color(255, 90, 160));
            btn.setOutlineThickness(3.f);
            btn.setOutlineColor(sf::Color::White);
        }
        else
        {
            btn.setFillColor(sf::Color(45, 30, 80, 220));
            btn.setOutlineThickness(2.f);
            btn.setOutlineColor(sf::Color(120, 100, 170));
        }
        win.draw(btn);

        middletxt(win, options[i], 24, sf::Color::White, WINDOW_W / 2.f, y - 2.f);
    }

    if (!menu_ins.empty())
    {
        middletxt(win, menu_ins, 20, sf::Color(255, 110, 110), WINDOW_W / 2.f, 600.f);
    }
}

// Mahshaf - Feature 1 & 7: draws current game state
void draw(sf::RenderWindow& win)
{
    static sf::CircleShape candy(CANDY_RADIUS);
    static sf::RectangleShape boardArea({ COLS * CELL, ROWS * CELL });

    switch (gameState)
    {
    case MENU:
        bg(win, sf::Color(70, 30, 110), sf::Color(20, 10, 40));
        bounce(win, candy);
        menu(win);
        break;

    case PLAYING:
        bg(win, sf::Color(30, 30, 60), sf::Color(10, 10, 25));

        boardArea.setPosition({ BOARD_X, BOARD_Y });
        boardArea.setFillColor(sf::Color(40, 40, 75));
        boardArea.setOutlineThickness(3.f);
        boardArea.setOutlineColor(sf::Color(90, 90, 140));
        win.draw(boardArea);

        drawBoard(win);
        hint_shuffle(win);
        hud(win);
        break;

    case GAME_OVER:
        if (playerWon)
            bg(win, sf::Color(120, 70, 20), sf::Color(40, 15, 50));
        else
            bg(win, sf::Color(110, 25, 35), sf::Color(30, 5, 10));
        endscr(win);
        break;
    }

    win.display();
}

// Mahshaf - Feature 1: main window and input -> update -> draw loop
int main()
{
    srand((unsigned)time(0));

    sf::RenderWindow window(sf::VideoMode({ WINDOW_W, WINDOW_H }), "Candy Crush");
    window.setFramerateLimit(60);

    fontLoaded = gameFont.openFromFile("game_font.ttf");

    while (window.isOpen())
    {
        input(window);
        update(window);
        draw(window);
    }

    return 0;
}