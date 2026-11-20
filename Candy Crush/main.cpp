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





// =================== FAIQA: level file, board drawing, clicks & swaps, save (F2, F3, F8) ===================

bool loadLevel(const string& filename)                 
{
    ifstream file(filename);
    if (!file.is_open())                                             // reads the numbers from level.txt into memory
    {
        return false;
    }

    for (int row = 0; row < ROWS; row++)
    {
        for (int col = 0; col < COLS; col++)
        {
            int value = 0;
            if (!(file >> value))
            {
                return false;
            }

            if (value < 0 || value > 6)
            {
                value = 0;
            }

            board[row][col] = value;
        }
    }

    file.close();
    return true;
}

//
//void drawBoard(sf::RenderWindow& win)
//{
//                                                     // //drawBoard bridges that data to the display 
//
//
//
//
//}


// =================== MAHSHAF: HUD, menu, hint & shuffle (F6, F7, Bonus) ===================

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

// YOUR TURN: write checkGameEnd() here (see the hints)

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
void updateGame()
{
    frameCount++;

    switch (gameState)
    {
    case MENU:
        // TEMPORARY: Enter starts a test game (real New Game + loadLevel come later)
        if (pressedKey == KEY_ENTER)
        {
            score = 0;
            movesLeft = 20;
            targetScore = 800;
            gameState = PLAYING;
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
    bool marked[ROWS][COLS] = {};

   
    while (window.isOpen())
    {
        handleInput(window);
        updateGame();
        drawGame(window);
    }
 

  

    return 0;
}