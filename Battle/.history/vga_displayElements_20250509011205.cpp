//==================================================
// BATTLESHIP_GAME - VGA DISPLAY ELEMENTS
//==================================================

#include "vga_displayElements.h"
#include "vga256_graphics.h"

// draw rect cell with left corner at (x,y)
void drawRect_grid(int x, int y, char color)
{
    drawHLine(x, y, GRID_SQUARE_SIZE, color);
    drawHLine(x, y + GRID_SQUARE_SIZE - 1, GRID_SQUARE_SIZE, color);
    drawVLine(x, y, GRID_SQUARE_SIZE, color);
    drawVLine(x + GRID_SQUARE_SIZE - 1, y, GRID_SQUARE_SIZE, color);
    for (int i = x + 1; i < (x + GRID_SQUARE_SIZE) - 1; i++)
    {
        for (int j = y + 1; j < (y + GRID_SQUARE_SIZE) - 1; j++)
        {
            drawPixel(i, j, GRID_FILL);
        }
    }
}
void drawBoundary()
{
    // draw boundary and 0,0
    // Step 0: draw basket as boundary
    drawLine(0, 0, 320, 0, WHITE);
    drawLine(320, 0, 320, 240, YELLOW);
    drawLine(0, 0, 0, 240, BLUE);
    drawLine(0, 240, 320, 240, RED);

    drawRect(0, 0, 10, 10, WHITE);
}

// Draw grid with left corner at (x,y)
void drawGRID(int Num, int x, int y, char outlineColor, char gridFill)
{
    for (int i = 0; i < Num; i++)
    {
        for (int j = 0; j < Num; j++)
            drawRect_grid(x + (j * GRID_SQUARE_SIZE), y + (i * GRID_SQUARE_SIZE), GRID_OUTLINE);
    }
}

// draw 10 by 10, grid dimension
// x and y is the the GRID left corner position for compatible
void drawGridDim(int grid_x, int grid_y, char gridFill)
{
    // change the grid x,y to the dimension's x and y
    int x = grid_x - 6;
    int y = grid_y + 1;

    //=============== vertical 1 to 10 ========================
    // 1 - 9
    for (int i = 0; i <= 8; i++)
    {
        drawChar(x, y + i * GRID_SQUARE_SIZE, 49 + i, gridFill, BLACK, 1);
    }

    // // 10
    static char ten[2];
    setTextColor(gridFill);
    setTextSize(1);

    setCursor(x - 6, y + 9 * GRID_SQUARE_SIZE);
    sprintf(ten, "10");
    writeString(ten);

    //=============== vertical 1 to 10 ========================
    for (int i = 0; i <= 9; i++)
    {
        drawChar(x + 8 + i * GRID_SQUARE_SIZE, y - 8, 65 + i, gridFill, BLACK, 1);
    }
}

// draw peg with center at (x,y)
void drawPEG(int x, int y, char color)
{
    fillCircle(x, y, PEG_RADIUS, color);
}

/*
Draw one peg at grid matrix (x,y)
ONly apply on left grid
*/
void drawPegShip(int x, int y)
{
    int peg_x, peg_y;
    peg_x = LEFT_GRID_X + (x * GRID_SQUARE_SIZE);
    peg_x = peg_x + 0.5 * GRID_SQUARE_SIZE;
    peg_y = LEFT_GRID_Y + (y * GRID_SQUARE_SIZE);
    peg_y = peg_y + 0.5 * GRID_SQUARE_SIZE;

    drawPEG(peg_x, peg_y, BLACK);
}

void drawPegPotentialShip(int x, int y)
{
    int peg_x, peg_y;
    peg_x = RIGHT_GRID_X + (x * GRID_SQUARE_SIZE);
    peg_x = peg_x + 0.5 * GRID_SQUARE_SIZE;
    peg_y = RIGHT_GRID_Y + (y * GRID_SQUARE_SIZE);
    peg_y = peg_y + 0.5 * GRID_SQUARE_SIZE;

    drawPEG(peg_x, peg_y, BLACK);
}

void drawPegHit(int x, int y)
{
    int peg_x, peg_y;
    peg_x = LEFT_GRID_X + (x * GRID_SQUARE_SIZE);
    peg_x = peg_x + 0.5 * GRID_SQUARE_SIZE;
    peg_y = LEFT_GRID_Y + (y * GRID_SQUARE_SIZE);
    peg_y = peg_y + 0.5 * GRID_SQUARE_SIZE;

    drawPEG(peg_x, peg_y, RED);
}

void drawPegMiss(int x, int y)
{
    int peg_x, peg_y;
    peg_x = LEFT_GRID_X + (x * GRID_SQUARE_SIZE);
    peg_x = peg_x + 0.5 * GRID_SQUARE_SIZE;
    peg_y = LEFT_GRID_Y + (y * GRID_SQUARE_SIZE);
    peg_y = peg_y + 0.5 * GRID_SQUARE_SIZE;

    drawPEG(peg_x, peg_y, GRAY2);
}

/*
This function is to draw the Welcome Page (Intial Page)
The next page is select difficulty or wait for another user to select difficulty

All text will be in the same "color". To clean the text, call the function with color = BLACK
*/
void welcomeText(char color)
{
    static char welcome[50];

    // printf("\nEntered HERe");
    // "Welcome to Battleship" The word will be in the middle of the screen vertically.
    setTextColor(color);
    setTextSize(4);

    setCursor(60, SCREEN_HEIGHT / 8);
    sprintf(welcome, "Welcome");
    writeString(welcome);

    setCursor(125, SCREEN_HEIGHT / 8 + 65);
    sprintf(welcome, "to");
    writeString(welcome);

    setCursor(SCREEN_WIDTH / 16, SCREEN_HEIGHT / 8 + 130);
    sprintf(welcome, "BATTLESHIP");
    writeString(welcome);

    // "Press BUTTON to start"
    setTextSize(1);
    setCursor(START_BUTTON_X - 36, START_BUTTON_Y);
    sprintf(welcome, "Press");
    writeString(welcome);

    setCursor(START_BUTTON_X, START_BUTTON_Y);
    sprintf(welcome, "BUTTON");
    writeString(welcome);

    setCursor(START_BUTTON_X + 46, START_BUTTON_Y);
    sprintf(welcome, "to Start");
    writeString(welcome);
}

/*
This function is to display difficulty selection page.
This function allows textcolor for "Good News! You will be the one to choose difficulty:"
                    optionColor and selected Option (0 - easy, 1 - difficult) and isShow (0-clean, 1-show)
When switch between easy and hard option, please simply call this function again with different selectedOption.
when clean all page, set isShow to be 0.
*/
void difficultyChoose(char textColor, char optionColor, int selectedOption, int isShow)
{

    char selected_bg = RED;
    char unselected_bg = BLACK;
    if (isShow == 0)
    {
        textColor = BLACK;
        optionColor = BLACK;
        selected_bg = BLACK;
    }

    setTextColor(textColor);
    setTextSize(3);
    static char difficulty[50];

    setCursor(80, SCREEN_HEIGHT / 8 + 8);

    sprintf(difficulty, "Good News! ");

    writeString(difficulty);

    setCursor(47, SCREEN_HEIGHT / 8 + 60);
    setTextSize(2);
    sprintf(difficulty, "You will be the one");
    writeString(difficulty);

    setCursor(37, SCREEN_HEIGHT / 8 + 90);
    setTextSize(2);
    sprintf(difficulty, "to choose difficulty:");
    writeString(difficulty);

    if (selectedOption == 0)
    {
        fillRect(59, SCREEN_HEIGHT / 8 + 139, 55, 17, unselected_bg); // changed to selected later
        fillRect(199, SCREEN_HEIGHT / 8 + 139, 55, 17, unselected_bg);
    }
    else
    {
        fillRect(59, SCREEN_HEIGHT / 8 + 139, 55, 17, unselected_bg);
        fillRect(199, SCREEN_HEIGHT / 8 + 139, 55, 17, unselected_bg);
    }

    setCursor(EASY_BUTTON_X, EASY_BUTTON_Y);
    setTextColor(optionColor);
    setTextSize(2);
    static char option1[50];
    sprintf(option1, "EASY");
    writeString(option1);

    setCursor(HARD_BUTTON_X, HARD_BUTTON_Y);
    setTextColor(optionColor);
    setTextSize(2);
    static char option2[50];
    sprintf(option2, "HARD");
    writeString(option2);
}

void drawCursor(int x, int y, char color)
{
    drawPixel(x, y, color);
    drawPixel(x - 1, y, color);
    drawPixel(x - 2, y, color);
    drawPixel(x + 1, y, color);
    drawPixel(x + 2, y, color);
    drawPixel(x, y - 1, color);
    drawPixel(x, y - 2, color);
    drawPixel(x, y + 1, color);
    drawPixel(x, y + 2, color);
}

PixelBackup prevPixels[CURSOR_SIZE];
PixelBackup currPixels[CURSOR_SIZE];

void setDefaultPixels()
{
    for (int i = 0; i < CURSOR_SIZE; i++)
    {
        prevPixels[i].x = 0;
        prevPixels[i].y = 0;
        prevPixels[i].color = BLACK; // assume you have this
    }
}

void savePrevPixels(fix15 x_fix, fix15 y_fix)
{
    int x = fix2int15(x_fix);
    int y = fix2int15(y_fix);

    int coords[CURSOR_SIZE][2] = {
        {x, y}, {x - 1, y}, {x - 2, y}, {x + 1, y}, {x + 2, y}, {x, y - 1}, {x, y - 2}, {x, y + 1}, {x, y + 2}};

    for (int i = 0; i < CURSOR_SIZE; i++)
    {
        int px = coords[i][0];
        int py = coords[i][1];
        currPixels[i].x = px;
        currPixels[i].y = py;
        currPixels[i].color = readPixel(px, py); // assume you have this
        // printf("\nThe color is: %u", currPixels[i].color);
    }
}

// void savePrevPixels(fix15 x_fix, fix15 y_fix)
// {
//     int x = fix2int15(x_fix);
//     int y = fix2int15(y_fix);

//     int coords[CURSOR_SIZE][2] = {
//         {x, y}, {x - 1, y}, {x - 2, y}, {x + 1, y}, {x + 2, y}, {x, y - 1}, {x, y - 2}, {x, y + 1}, {x, y + 2}};

//     for (int i = 0; i < CURSOR_SIZE; i++)
//     {
//         int px = coords[i][0];
//         int py = coords[i][1];
//         prevPixels[i].x = px;
//         prevPixels[i].y = py;
//         prevPixels[i].color = readPixel(px, py); // assume you have this
//         // printf("\nThe color is: %u", currPixels[i].color);
//     }
// }

void moveCursor(fix15 *old_x, fix15 *old_y, fix15 new_x, fix15 new_y, char cursor_color)
{

    // Save pixels under new location
    savePrevPixels(new_x, new_y);
    // for(int i=0;i<CURSOR_SIZE; i++)
    // {
    //     printf("\nCurrPixel[%d]: x:%d, y:%d, color:%d", i, currPixels[i].x, currPixels[i].y, currPixels[i].color);
    // }

    // Restore previous pixels
    for (int i = 0; i < CURSOR_SIZE; i++)
    {
        // printf("\nPrevPixel[%d]: x:%d, y:%d, color:%d", i, prevPixels[i].x, prevPixels[i].y, prevPixels[i].color);
        drawPixel(prevPixels[i].x, prevPixels[i].y, prevPixels[i].color);
        prevPixels[i].x = currPixels[i].x;
        prevPixels[i].y = currPixels[i].y;
        prevPixels[i].color = currPixels[i].color;
    }

    // Draw new cursor
    drawCursor(fix2int15(new_x), fix2int15(new_y), cursor_color);

    // update old cursor
    *old_x = new_x;
    *old_y = new_y;
}

// void moveCursor(fix15 *old_x, fix15 *old_y, fix15 new_x, fix15 new_y, char cursor_color)
// {

//     for (int i = 0; i < CURSOR_SIZE; i++) {
//         // printf("\nPrevPixel[%d]: x:%d, y:%d, color:%d", i, prevPixels[i].x, prevPixels[i].y, prevPixels[i].color);
//         drawPixel(prevPixels[i].x, prevPixels[i].y, prevPixels[i].color);

//         prevPixels[i].x = currPixels[i].x;
//         prevPixels[i].y = currPixels[i].y;
//         prevPixels[i].color = currPixels[i].color;

//     }

//     // Save pixels under new location
//     savePrevPixels(new_x, new_y);

//     drawCursor(fix2int15(new_x), fix2int15(new_y), cursor_color);

//     // update old cursor
//     *old_x = new_x;
//     *old_y = new_y;
// }

bool checkCursorOverStartButton(int x_pos, int y_pos)
{
    if (x_pos >= START_BUTTON_X && x_pos <= START_BUTTON_X + START_BUTTON_WIDTH && y_pos >= START_BUTTON_Y && y_pos <= START_BUTTON_Y + START_BUTTON_HEIGHT)
        return true;
    else
        return false;
}

uint8_t checkCursorOverLevel(int x_pos, int y_pos)
{
    if (x_pos >= EASY_BUTTON_X && x_pos <= EASY_BUTTON_X + START_BUTTON_WIDTH && y_pos >= EASY_BUTTON_Y && y_pos <= EASY_BUTTON_Y + START_BUTTON_HEIGHT)
        return 1;
    else if (x_pos >= HARD_BUTTON_X && x_pos <= HARD_BUTTON_X + START_BUTTON_WIDTH && y_pos >= HARD_BUTTON_Y && y_pos <= HARD_BUTTON_Y + START_BUTTON_HEIGHT)
        return 2;
    else
        return 0;
}

bool checkCursorOverStartGame(int x_pos, int y_pos)
{
    if (x_pos >= START_GAME_X && x_pos <= START_GAME_X + 120 && y_pos >= START_GAME_Y && y_pos <= START_GAME_Y + 20)
        return true;
    else
        return false;
}

void drawTextforShip(char textColor, char optionColor, int isShow)
{

    char selected_bg = RED;
    char unselected_bg = BLACK;
    if (isShow == 0)
    {
        textColor = BLACK;
        optionColor = BLACK;
        selected_bg = BLACK;
    }

    setTextColor(textColor);
    setTextSize(2);
    static char shipslist[50];

    setCursor(RIGHT_GRID_X, RIGHT_GRID_Y);
    sprintf(shipslist, "Ship List! ");
    writeString(shipslist);

    setCursor(RIGHT_GRID_X, SHIPLIST_SPACE_Carrier);
    setTextSize(1);
    sprintf(shipslist, "Carrier    (5)");
    writeString(shipslist);

    setCursor(RIGHT_GRID_X, SHIPLIST_SPACE_Battleship);
    setTextSize(1);
    sprintf(shipslist, "Battleship (4)");
    writeString(shipslist);

    setCursor(RIGHT_GRID_X, SHIPLIST_SPACE_Cruiser);
    setTextSize(1);
    sprintf(shipslist, "Cruiser    (3)");
    writeString(shipslist);

    setCursor(RIGHT_GRID_X, SHIPLIST_SPACE_Submarine);
    setTextSize(1);
    sprintf(shipslist, "Submarine  (3)");
    writeString(shipslist);

    setCursor(RIGHT_GRID_X, SHIPLIST_SPACE_Destroyer);
    setTextSize(1);
    sprintf(shipslist, "Destroyer  (2)");
    writeString(shipslist);

    setCursor(20, EASY_BUTTON_Y);
    setTextColor(optionColor);
    setTextSize(2);
    static char option1[50];
    sprintf(option1, "START GAME");
    writeString(option1);
}

uint8_t checkCursorOverShip(int x_pos, int y_pos)
{
    if (x_pos >= RIGHT_GRID_X - 1 && x_pos <= RIGHT_GRID_X - 1 + START_BUTTON_WIDTH &&
        y_pos >= SHIPLIST_SPACE_Carrier - 1 && y_pos <= SHIPLIST_SPACE_Carrier - 1 + START_BUTTON_HEIGHT / 2)
        return 1;
    else if (x_pos >= RIGHT_GRID_X - 1 && x_pos <= RIGHT_GRID_X - 1 + START_BUTTON_WIDTH &&
             y_pos >= SHIPLIST_SPACE_Battleship - 1 && y_pos <= SHIPLIST_SPACE_Battleship - 1 + START_BUTTON_HEIGHT / 2)
        return 2;
    else if (x_pos >= RIGHT_GRID_X - 1 && x_pos <= RIGHT_GRID_X - 1 + START_BUTTON_WIDTH &&
             y_pos >= SHIPLIST_SPACE_Cruiser - 1 && y_pos <= SHIPLIST_SPACE_Cruiser - 1 + START_BUTTON_HEIGHT / 2)
        return 3;
    else if (x_pos >= RIGHT_GRID_X - 1 && x_pos <= RIGHT_GRID_X - 1 + START_BUTTON_WIDTH &&
             y_pos >= SHIPLIST_SPACE_Submarine - 1 && y_pos <= SHIPLIST_SPACE_Submarine - 1 + START_BUTTON_HEIGHT / 2)
        return 4;
    else if (x_pos >= RIGHT_GRID_X - 1 && x_pos <= RIGHT_GRID_X - 1 + START_BUTTON_WIDTH &&
             y_pos >= SHIPLIST_SPACE_Destroyer - 1 && y_pos <= SHIPLIST_SPACE_Destroyer - 1 + START_BUTTON_HEIGHT / 2)
        return 5;
    else if (x_pos >= EASY_BUTTON_X - 1 && x_pos <= EASY_BUTTON_X - 1 + START_BUTTON_WIDTH &&
             y_pos >= EASY_BUTTON_Y - 1 && y_pos <= EASY_BUTTON_Y - 1 + START_BUTTON_HEIGHT / 2)
        return 1;
    else
        return 0;
}

void drawBoxforShip(char color, int pos_x, int pos_y, const char *strin)
{
    static char shipslist[50];
    fillRect(pos_x - 1, pos_y - 3, 118, 13, color);
    setCursor(pos_x, pos_y);
    setTextSize(1);
    setTextColor(BLACK); // default is white but not very easy to see
    sprintf(shipslist, strin);
    writeString(shipslist);
}

void drawStartGame(char bgColor)
{
    static char shipslist[50];
    fillRect(START_GAME_X - 1, START_GAME_Y - 3, 120, 20, bgColor);
    setCursor(START_GAME_X, START_GAME_Y);
    setTextSize(1);
    setTextColor(BLACK); // default is white but not very easy to see
    sprintf(shipslist, strin);
    writeString(shipslist);
}

void drawBlackBoxforShip()
{
    fillRect(RIGHT_GRID_X - 1, RIGHT_GRID_Y - 3, 118, 13, BLACK);
    fillRect(RIGHT_GRID_X - 1, SHIPLIST_SPACE_Carrier - 3, 118, 13, BLACK);
    fillRect(RIGHT_GRID_X - 1, SHIPLIST_SPACE_Battleship - 3, 118, 13, BLACK);
    fillRect(RIGHT_GRID_X - 1, SHIPLIST_SPACE_Cruiser - 3, 118, 13, BLACK);
    fillRect(RIGHT_GRID_X - 1, SHIPLIST_SPACE_Submarine - 3, 118, 13, BLACK);
    fillRect(RIGHT_GRID_X - 1, SHIPLIST_SPACE_Destroyer - 3, 118, 13, BLACK);
}
/*
This function is to display waiting for difficulty selection page.
This function allows textcolor for the entire page and can set to BLACK to clean it
*/
void waitDifficultyChoose(char textColor)
{
    setTextColor(textColor);
    setTextSize(2);
    static char difficulty[50];

    setCursor(25, SCREEN_HEIGHT / 8 + 10);
    sprintf(difficulty, "Hey! Your opponent is");
    writeString(difficulty);

    setCursor(110, SCREEN_HEIGHT / 8 + 40);
    sprintf(difficulty, "choosing");
    writeString(difficulty);

    setCursor(35, SCREEN_HEIGHT / 8 + 70);
    sprintf(difficulty, "the difficulty level.");
    writeString(difficulty);

    setCursor(75, SCREEN_HEIGHT / 8 + 120);
    sprintf(difficulty, "Please wait :)");
    writeString(difficulty);
}

/*
Winner/Loser declaration

For winner: BLACK textColor will clear the entire text and image on the screen
*/
void winnerDeclare(char textColor)
{
    setTextColor(textColor);
    setTextSize(4);
    static char declare[50];

    setCursor(30, SCREEN_HEIGHT / 8 + 25);
    sprintf(declare, "YOU WIN!!!");
    writeString(declare);
    // if (textColor != BLACK)
    // {
    // imageDraw(120, 110, trophy_xmax, trophy_ymax, trophy_data, true);
    // }
    // else
    // {
    // imageDraw(120, 110, trophy_xmax, trophy_ymax, trophy_data, false);
    // }
}

void loserDeclare(char textColor)
{
    setTextColor(textColor);
    setTextSize(4);
    static char declare[50];

    setCursor(30, SCREEN_HEIGHT / 8 + 25);
    sprintf(declare, "YOU LOSE...");
    writeString(declare);
}

/*This function is to draw image on VGA.
INPUTS:
    image_x_offset/image_y_offset: coordinate of the left upper corner
    xmax/ymax: the coordinate of image data array
    data[][xmax]: image data array
*/
/*
void imageDraw(short image_x_offset, short image_y_offset, short xmax, short ymax, const uint8_t data[][xmax], bool dispaly)
{
    short draw_x, draw_y;
    uint8_t draw_cmd;

    draw_x = image_x_offset;
    draw_y = image_y_offset - 1;

    for (int y = 0; y < ymax; y++)
    {
        for (int x = 0; x < xmax; x++)
        {
            //
            if (x == 0)
            {
                draw_y = draw_y + 1;
                draw_x = image_x_offset;
            }

            draw_x = draw_x + 1;

            if (dispaly)
            {
                draw_cmd = data[y][x]; // actually the image data
            }
            else
            {
                draw_cmd = BLACK;
            }

            drawPixel(draw_x, draw_y, draw_cmd);
        }
    }
}
*/