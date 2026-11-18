// =====================================================================
//  Team IMF - Candy Crush  (CS1002 Programming Fundamentals, Fall 2026)
//  Feature 1: game window and the input -> update -> draw game loop
// =====================================================================

#include <SFML/Graphics.hpp>
#include <optional>
#include <cstdlib>
#include <ctime>
#include <cmath>
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

// ---------------- Input of the current frame ----------------
int clickX = 0;
int clickY = 0;
bool clicked = false;
int pressedKey = KEY_NONE;

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
        // TEMPORARY: Enter starts playing (the real menu comes in Feature 7)
        if (pressedKey == KEY_ENTER)
            gameState = PLAYING;
        break;

    case PLAYING:
        // TEMPORARY: Esc goes back to the menu (saving comes in Feature 8)
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
    // shapes are made once (static) and only moved/recoloured every frame
    static sf::CircleShape candy(CANDY_RADIUS);
    static sf::RectangleShape hud({ (float)WINDOW_W, (float)HUD_HEIGHT });
    static sf::RectangleShape hudLine({ (float)WINDOW_W, 3.f });
    static sf::RectangleShape boardArea({ COLS * CELL, ROWS * CELL });

    switch (gameState)
    {
    case MENU:
        drawBackground(win, sf::Color(70, 30, 110), sf::Color(20, 10, 40));
        drawBouncingCandies(win, candy);
        break;

    case PLAYING:
        drawBackground(win, sf::Color(30, 30, 60), sf::Color(10, 10, 25));

        hud.setFillColor(sf::Color(20, 20, 45, 230));        // HUD strip at the top
        win.draw(hud);

        hudLine.setPosition({ 0.f, (float)HUD_HEIGHT - 3.f }); // pink accent line under the HUD
        hudLine.setFillColor(sf::Color(255, 90, 160));
        win.draw(hudLine);

        boardArea.setPosition({ BOARD_X, BOARD_Y });          // where the board will be drawn
        boardArea.setFillColor(sf::Color(40, 40, 75));
        boardArea.setOutlineThickness(3.f);
        boardArea.setOutlineColor(sf::Color(90, 90, 140));
        win.draw(boardArea);
        break;

    case GAME_OVER:
        drawBackground(win, sf::Color(110, 25, 35), sf::Color(30, 5, 10));
        break;
    }

    win.display();
}

int main()
{
    srand(time(0));

    sf::RenderWindow window(sf::VideoMode({ WINDOW_W, WINDOW_H }), "Candy Crush");
    window.setFramerateLimit(60);   // same speed on every laptop

    // The game loop: input, update, draw - always in this order.
    while (window.isOpen())
    {
        handleInput(window);
        updateGame();
        drawGame(window);
    }

    return 0;
}