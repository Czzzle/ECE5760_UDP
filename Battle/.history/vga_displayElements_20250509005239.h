//==================================================
// BATTLESHIP_GAME - VGA DISPLAY ELEMENTS
//==================================================

#ifndef VGA_DISPLAYELEMENTS_H
#define VGA_DISPLAYELEMENTS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"

#include "vga256_graphics.h"
#include "vga_displayElements.h"

#include "fix15.h"

#include "trophyData.h"

// --------------------------------------------------------------------
#define SCREEN_WIDTH 320 // Physical VGA screen
#define SCREEN_HEIGHT 240

#define GRID_SIZE 5         // 10        // Full grid is 10 by 10
#define GRID_SQUARE_SIZE 10 // 20 // Cell size is 20 pixel *20 pixel
#define GRID_OUTLINE BLACK
#define GRID_FILL CYAN

#define LEFT_GRID_X SCREEN_WIDTH / 10  // 5
#define LEFT_GRID_Y SCREEN_HEIGHT / 10 // 5
#define RIGHT_GRID_X (SCREEN_WIDTH / 10) + GRID_SIZE *GRID_SQUARE_SIZE + 80
#define RIGHT_GRID_Y LEFT_GRID_Y

#define PEG_COLOR RED
#define PEG_RADIUS GRID_SQUARE_SIZE / 5

#define START_BUTTON_X      186
#define START_BUTTON_Y      (SCREEN_HEIGHT / 8 + 190)
#define START_BUTTON_WIDTH  60   // Adjust as needed
#define START_BUTTON_HEIGHT 20   // Adjust as needed

#define EASY_BUTTON_X   60
#define EASY_BUTTON_Y   (SCREEN_HEIGHT / 8 + 140)
#define HARD_BUTTON_X   200
#define HARD_BUTTON_Y   (SCREEN_HEIGHT / 8 + 140)

#define SHIPLIST_SPACE_Carrier RIGHT_GRID_Y + 30
#define SHIPLIST_SPACE_Battleship RIGHT_GRID_Y + 60
#define SHIPLIST_SPACE_Cruiser RIGHT_GRID_Y + 90
#define SHIPLIST_SPACE_Submarine RIGHT_GRID_Y + 120
#define SHIPLIST_SPACE_Destroyer RIGHT_GRID_Y + 150

#define START_GAME_X 20
#

#define CURSOR_SIZE 9

// Page 1 - WELCOME
void welcomeText(char color);
void setDefaultPixels();
void drawCursor(int x, int y, char color);
typedef struct {
    int x, y;
    char color;
} PixelBackup;

void savePrevPixels(fix15 x, fix15 y);
void moveCursor(fix15 *old_x, fix15 *old_y, fix15 new_x, fix15 new_y, char cursor_color);


// Page 2 - Level
void difficultyChoose(char textColor, char optionColor, int selectedOption, int isShow);
void waitDifficultyChoose(char textColor);

// Page 3 - GRID
void drawRect_grid(int x, int y, char color);
void drawBoundary();
void drawGRID(int Num, int x, int y, char outlineColor, char gridFill);
void drawGridDim(int grid_x, int grid_y, char gridFill);
void drawPEG(int x, int y, char color);
void drawPegShip(int x, int y);
void drawPegPotentialShip(int x, int y);
void drawTextforShip(char textColor, char optionColor, int isShow);
void drawBoxforShip(char color, int pos_x, int pos_y, const char* strin);
void drawBlackBoxforShip();

// Page 4 - Game
void drawPegHit(int x, int y);
void drawPegMiss(int x, int y);

// Page 5 - Result
void winnerDeclare(char textColor);
void loserDeclare(char textColor);

//Checkers
bool checkCursorOverStartButton(int x_pos, int y_pos);
uint8_t checkCursorOverLevel(int x_pos, int y_pos);
uint8_t checkCursorOverShip(int x_pos, int y_pos);
bool checkCursorOverStartGame(int x_pos, int y_pos);
// helper function
// void imageDraw(short image_x_offset, short image_y_offset, short xmax, short ymax, const uint8_t data[][xmax], bool dispaly);

#endif /* VGA_DISPLAYELEMENTS_H */