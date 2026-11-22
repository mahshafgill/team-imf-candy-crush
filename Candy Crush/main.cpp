// =====================================================================
//  Team IMF - Candy Crush  (CS1002 Programming Fundamentals, Fall 2026)
//  Feature 1: game window and the input -> update -> draw game loop
// =====================================================================

#include <SFML/Graphics.hpp>
#include <optional>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <string>
#include<fstream>
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
int frameCount = 0;                 // counts frames; used for animations (and later the hint timer)
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
    for (int r = 0;r < ROWS;r++)
    {
        for (int c = 0;c < COLS;c++)
        {
            marked[r][c] = false;
        }
    }
    
    for (int r = 0;r < ROWS;r++)//ROWS
    {
        for (int c = 0;c < COLS-2;c++)
        {
            if (board[r][c] == board[r][c + 1] && board[r][c] == board[r][c + 2] && board[r][c] != 0)
            {
                marked[r][c] = true;
                marked[r][c+1] = true;
                marked[r][c+2] = true;
            }
        }
    }
    for (int c = 0;c < COLS;c++)//COLS
    {
        for (int r = 0;r < ROWS-2;r++)
        {
            if (board[r][c]==board[r+1][c]&& board[r][c] == board[r + 2][c]&&board[r][c]!=0)
            {
                marked[r][c] = true;
                marked[r+1][c] = true;
                marked[r+2][c] = true;


            }
        }
    }
    for (int r = 0;r < ROWS;r++)
    {
        for (int c = 0;c < COLS;c++)
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
    for (int r = 0;r < ROWS;r++)
    {
        for (int c = 0;c < COLS;c++)
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

// =================== MAHSHAF: HUD, menu, hint & shuffle (F6, F7, Bonus) ===================

int menuSelection = 0;              // 0: New Game, 1: Continue, 2: Exit
string menuMessage = "";            // displays "No saved game" or missing level error

// Forward declarations
bool loadSavedGame();
void activateMenuOption(sf::RenderWindow& win);

// Temporary stub until Faiqa adds Feature 8 (savegame.txt)
bool loadSavedGame()
{
    return false;
}

// Draws text so that its centre is at (x, y). Used for every label in the HUD and menus.
void drawCenteredText(sf::RenderWindow& win, const string& str, unsigned int size,
    sf::Color color, float x, float y)
{
    if (!fontLoaded)
        return;                         // no font file: skip the text instead of crashing

    sf::Text text(gameFont, str, size);
    text.setFillColor(color);
    sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin({ bounds.position.x + bounds.size.x / 2.f,
                     bounds.position.y + bounds.size.y / 2.f });
    text.setPosition({ x, y });
    win.draw(text);
}

// Feature 6: the 80-pixel strip at the top showing SCORE | MOVES | TARGET and a progress bar.
void drawHUD(sf::RenderWindow& win)
{
    sf::RectangleShape hud({ (float)WINDOW_W, (float)HUD_HEIGHT });
    hud.setFillColor(sf::Color(20, 20, 45, 230));
    win.draw(hud);

    float colW = WINDOW_W / 3.f;                    // three equal columns
    sf::Color labelColor(170, 170, 210);

    drawCenteredText(win, "SCORE", 14, labelColor, colW * 0.5f, 20.f);
    drawCenteredText(win, to_string(score), 30, sf::Color::White, colW * 0.5f, 46.f);

    sf::Color movesColor = sf::Color::White;
    if (movesLeft <= 5)
        movesColor = sf::Color(255, 90, 90);        // red warning when few moves are left
    drawCenteredText(win, "MOVES", 14, labelColor, colW * 1.5f, 20.f);
    drawCenteredText(win, to_string(movesLeft), 30, movesColor, colW * 1.5f, 46.f);

    drawCenteredText(win, "TARGET", 14, labelColor, colW * 2.5f, 20.f);
    drawCenteredText(win, to_string(targetScore), 30, sf::Color(255, 215, 90), colW * 2.5f, 46.f);

    // progress bar: how close the score is to the target (0 = empty, 1 = full)
    float progress = 0.f;
    if (targetScore > 0)
        progress = (float)score / targetScore;
    if (progress > 1.f)
        progress = 1.f;

    sf::RectangleShape barBack({ (float)WINDOW_W, 6.f });
    barBack.setPosition({ 0.f, HUD_HEIGHT - 6.f });
    barBack.setFillColor(sf::Color(60, 40, 80));
    win.draw(barBack);

    sf::RectangleShape barFill({ WINDOW_W * progress, 6.f });
    barFill.setPosition({ 0.f, HUD_HEIGHT - 6.f });
    barFill.setFillColor(sf::Color(255, 90, 160));
    win.draw(barFill);
}

// The "YOU WIN" / "GAME OVER" screen with the final score.
void drawEndScreen(sf::RenderWindow& win)
{
    if (playerWon)
        drawCenteredText(win, "YOU WIN!", 64, sf::Color(255, 215, 90), WINDOW_W / 2.f, 260.f);
    else
        drawCenteredText(win, "GAME OVER", 64, sf::Color(255, 90, 90), WINDOW_W / 2.f, 260.f);

    drawCenteredText(win, "Final score: " + to_string(score), 30, sf::Color::White, WINDOW_W / 2.f, 340.f);

    // "Press Enter" fades in and out: sin() gives -1..1, so (sin + 1) / 2 gives 0..1
    float pulse = (sin(frameCount * 0.1f) + 1.f) / 2.f;
    sf::Color fade(255, 255, 255, (std::uint8_t)(120 + pulse * 135));
    drawCenteredText(win, "Press Enter", 22, fade, WINDOW_W / 2.f, 430.f);
}

// Checks if the game is finished: target reached = win, no moves left = lose.
void checkGameEnd()
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

// Reads all events of this frame and stores them in the input variables above.
void handleInput(sf::RenderWindow& win)
{
    // forget last frame's input
    clicked = false;
    pressedKey = KEY_NONE;

    while (const std::optional event = win.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
        {
            win.close();            // X button ends the program
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

// Changes the game using this frame's input. Each state only does its own work.
void updateGame(sf::RenderWindow& win)
{
    frameCount++;

    switch (gameState)
    {
    case MENU:
        if (pressedKey == KEY_UP)
            menuSelection = (menuSelection + 2) % 3;
        else if (pressedKey == KEY_DOWN)
            menuSelection = (menuSelection + 1) % 3;
        else if (pressedKey == KEY_ENTER)
            activateMenuOption(win);

        if (clicked)
        {
            for (int i = 0; i < 3; i++)
            {
                float y = 390.f + i * 65.f;
                sf::FloatRect box({ WINDOW_W / 2.f - 120.f, y - 24.f }, { 240.f, 48.f });
                if (box.contains({ (float)clickX, (float)clickY }))
                {
                    menuSelection = i;
                    activateMenuOption(win);
                }
            }
        }
        break;

    case PLAYING:
        // TEMPORARY test keys: Up = +50 score, Down = use one move
        if (pressedKey == KEY_UP)
            score += 50;
        if (pressedKey == KEY_DOWN)
            movesLeft--;

        if (pressedKey == KEY_ESCAPE)
            gameState = MENU;

        checkGameEnd();             // win or lose? (checked after every change)
        break;

    case GAME_OVER:
        if (pressedKey == KEY_ENTER)
            gameState = MENU;
        break;
    }
}

// Fills the whole window with a smooth top-to-bottom colour gradient.
void drawBackground(sf::RenderWindow& win, sf::Color top, sf::Color bottom)
{
    sf::VertexArray bg(sf::PrimitiveType::TriangleStrip, 4);
    bg[0].position = { 0.f, 0.f };                              bg[0].color = top;
    bg[1].position = { (float)WINDOW_W, 0.f };                  bg[1].color = top;
    bg[2].position = { 0.f, (float)WINDOW_H };                  bg[2].color = bottom;
    bg[3].position = { (float)WINDOW_W, (float)WINDOW_H };      bg[3].color = bottom;
    win.draw(bg);
}

// Menu decoration: one candy of every colour, gently bouncing up and down.
void drawBouncingCandies(sf::RenderWindow& win, sf::CircleShape& candy)
{
    for (int i = 1; i <= 6; i++)
    {
        // sin() makes a smooth wave; each candy starts at a different point of the wave
        float bounce = sin(frameCount * 0.08f + i * 0.6f) * 10.f;
        candy.setFillColor(candyColors[i]);
        candy.setPosition({ 60.f + (i - 1) * 85.f, 300.f + bounce });
        win.draw(candy);
    }
}

// Runs the selected Main Menu option: New Game (0), Continue (1), or Exit (2)
void activateMenuOption(sf::RenderWindow& win)
{
    menuMessage = "";

    if (menuSelection == 0)         // New Game: always starts fresh from level.txt
    {
        if (loadLevel("level.txt"))
        {
            gameState = PLAYING;
        }
        else
        {
            menuMessage = "Error: level.txt not found!";
        }
    }
    else if (menuSelection == 1)    // Continue: load saved game
    {
        if (loadSavedGame())
        {
            selRow = -1;
            selCol = -1;
            gameState = PLAYING;
        }
        else
        {
            menuMessage = "No saved game";
        }
    }
    else if (menuSelection == 2)    // Exit
    {
        win.close();
    }
}

// Feature 7: Draws the Main Menu with New Game, Continue, Exit in exact order
void drawMenu(sf::RenderWindow& win)
{
    drawCenteredText(win, "CANDY CRUSH", 54, sf::Color(255, 215, 90), WINDOW_W / 2.f, 145.f);
    drawCenteredText(win, "Team IMF  |  CS1002", 18, sf::Color(180, 180, 220), WINDOW_W / 2.f, 195.f);

    const string options[3] = { "New Game", "Continue", "Exit" };
    float startY = 390.f;
    float gapY = 65.f;
    float btnW = 240.f;
    float btnH = 48.f;

    for (int i = 0; i < 3; i++)
    {
        float y = startY + i * gapY;
        bool isSelected = (i == menuSelection);

        sf::RectangleShape btn({ btnW, btnH });
        btn.setOrigin({ btnW / 2.f, btnH / 2.f });
        btn.setPosition({ WINDOW_W / 2.f, y });

        if (isSelected)
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

        drawCenteredText(win, options[i], 24, sf::Color::White, WINDOW_W / 2.f, y - 2.f);
    }

    if (!menuMessage.empty())
    {
        drawCenteredText(win, menuMessage, 20, sf::Color(255, 110, 110), WINDOW_W / 2.f, 600.f);
    }
}

// Draws the current state.
void drawGame(sf::RenderWindow& win)
{
    static sf::CircleShape candy(CANDY_RADIUS);
    static sf::RectangleShape boardArea({ COLS * CELL, ROWS * CELL });

    switch (gameState)
    {
    case MENU:
        drawBackground(win, sf::Color(70, 30, 110), sf::Color(20, 10, 40));
        drawBouncingCandies(win, candy);
        drawMenu(win);
        break;

    case PLAYING:
        drawBackground(win, sf::Color(30, 30, 60), sf::Color(10, 10, 25));

        boardArea.setPosition({ BOARD_X, BOARD_Y });   // where the board will be drawn
        boardArea.setFillColor(sf::Color(40, 40, 75));
        boardArea.setOutlineThickness(3.f);
        boardArea.setOutlineColor(sf::Color(90, 90, 140));
        win.draw(boardArea);

        drawHUD(win);
        break;

    case GAME_OVER:
        if (playerWon)
            drawBackground(win, sf::Color(120, 70, 20), sf::Color(40, 15, 50));   // gold
        else
            drawBackground(win, sf::Color(110, 25, 35), sf::Color(30, 5, 10));    // red
        drawEndScreen(win);
        break;
    }

    win.display();
}

int main()
{
    srand((unsigned)time(0));

    sf::RenderWindow window(sf::VideoMode({ WINDOW_W, WINDOW_H }), "Candy Crush");
    window.setFramerateLimit(60);   // same speed on every laptop

    // load the font once; if it's missing the game still runs, just without text
    fontLoaded = gameFont.openFromFile("game_font.ttf");

    // The game loop: input, update, draw - always in this order.
    

   
    while (window.isOpen())
    {
        handleInput(window);
        updateGame(window);
        drawGame(window);
    }
 

  

    return 0;
}