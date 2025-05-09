//==================================================
// BATTLESHIP_GAME
//==================================================

#ifndef BATTLESHIP_GAME_H
#define BATTLESHIP_GAME_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <cstring>
#include <unordered_map>
#include <vector>
#include "fix15.h"

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

//////////////////////////////////////////////////////////////////////////////
// ----------------------- GAME BOARD - OCEAN --------------------------------
//////////////////////////////////////////////////////////////////////////////

#define BOARD_SIZE 10 // NxN SQUARE Grid (N=10)

enum class GAME_STATUS { INITIAL,LEVEL,PLACE, ONGOING, WIN, LOSE };
enum class GRID_STATE { WATER, SHIP, HIT, MISS, REPEAT};

//////////////////////////////////////////////////////////////////////////////
// ----------------------- COORDINATE -------------------------------------------
//////////////////////////////////////////////////////////////////////////////

template <typename T>
struct Coordinate { T x; T y; };

#define Coordinate8 Coordinate<uint8_t>
#define Coordinatefix15 Coordinate<fix15>

// EQUAL TO
template <typename T>
inline bool operator==(const Coordinate<T> &lhs, const Coordinate<T> &rhs) {
    return lhs.x == rhs.x && lhs.y == rhs.y; }

// NOT EQUAL TO
template <typename T>
inline bool operator!=(const Coordinate<T> &lhs, const Coordinate<T> &rhs) {
    return lhs.x != rhs.x || lhs.y != rhs.y; }

// COUT OVERLOAD FOR Coordinate
template <typename T>
std::ostream &operator<<(std::ostream &os, const Coordinate<T> &coord) {
    os << "(" << (int)coord.x << ", " << (int)coord.y << ")";
    return os; }

//////////////////////////////////////////////////////////////////////////////
// -------------------------- DECODER ----------------------------------------
//////////////////////////////////////////////////////////////////////////////

// Coordinatefix15 CursorPosition;
// drawCursor();
// starts at (x,y)...joystick mover

Coordinate8 decodeCoord(const char *code);
void encodeCoord(Coordinate8 c, char *out);


//////////////////////////////////////////////////////////////////////////////
// ----------------------- SHIP - CLASS  -------------------------------------
//////////////////////////////////////////////////////////////////////////////

enum class SHIP_ORIENTATION { HORIZONTAL, VERTICAL };
enum class SHIP_TYPE { Carrier, Battleship, Cruiser, Submarine, Destroyer };

extern const std::unordered_map<SHIP_TYPE, std::pair<std::string, uint8_t>> SHIP_INFO;

// for each ship- we can increase or decrease # of ships easily if modularized.
class SHIP
{
    public:
        SHIP_TYPE type;
        SHIP_ORIENTATION orientation;
        Coordinate8 start;
        std::vector<Coordinate8> positions; // we will use to keep track of where the ship is
        std::vector<bool> hits;

        SHIP(SHIP_TYPE t, SHIP_ORIENTATION o, Coordinate8 s);
        bool is_sunk();
};

//////////////////////////////////////////////////////////////////////////////
// ----------------------- GAME BOARD ---------------------------------------
//////////////////////////////////////////////////////////////////////////////

class GameBoard {
    private: // keeping them in private because we dont want others to know about these details
        GRID_STATE board[BOARD_SIZE][BOARD_SIZE] = {{GRID_STATE::WATER}}; // Inititally setting all grid locations to water
        std::vector<SHIP> ships;

    public:
        bool place_ship(SHIP_TYPE type, SHIP_ORIENTATION orientation, Coordinate8 start);
        GRID_STATE attack(Coordinate8& target); // return the changes to the grid location
        bool all_ships_sunk(); // END OF Game condition
        GAME_STATUS game_status = GAME_STATUS::INITIAL; 
        uint8_t game_status_check()
        {
            if(game_status == GAME_STATUS::INITIAL)
                return 0;
            else if(game_status == GAME_STATUS::LEVEL)
                return 1;
            else if(game_status == GAME_STATUS::PLACE)
                return 2;
            else if(game_status == GAME_STATUS::ONGOING)
                return 3;
            else if(game_status == GAME_STATUS::WIN)
                return 4;
            else
                return 5;
        }  
};

#endif /* BATTLESHIP_GAME_H */