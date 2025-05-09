//==================================================
// Coordinate
//==================================================

#include "BattleshipGame.h"
// #include "vga_displayElements.h"
#include "vga_displayElements.h"
// #include "vga256_graphics.h"
#include <iostream>

#define FRAME_RATE 33000

using namespace std;

//////////////////////////////////////////////////////////////////////////////
// ----------------------- DECODER----  -------------------------------------
//////////////////////////////////////////////////////////////////////////////

// Decodes a coordinate from "A5" → (0,4)
Coordinate8 decodeCoord(const char* code) {
    Coordinate8 c;
    c.x = toupper(code[0]) - 'A';
    c.y = atoi(code + 1) - 1;
    return c;
}

// Encodes a coordinate from (0,4) → "A5"
void encodeCoord(Coordinate8 c, char* out) {
    out[0] = 'A' + c.x;
    sprintf(out + 1, "%d", c.y + 1);
}

// Returns the grid cell (Coordinate8) corresponding to a pixel coordinate (pixel_x, pixel_y).
Coordinate8 gridCellIdentifier(int pixel_x, int pixel_y) {
    Coordinate8 cell = {0, 0};

    // Adjust pixel_x and pixel_y relative to the grid origin
    int relative_x = pixel_x - LEFT_GRID_X;
    int relative_y = pixel_y - LEFT_GRID_Y;

    // If the cursor is outside the grid area, clamp to grid limits
    if (relative_x < 0) relative_x = 0;
    if (relative_y < 0) relative_y = 0;
    if (relative_x >= GRID_SIZE * GRID_SQUARE_SIZE) relative_x = (GRID_SIZE * GRID_SQUARE_SIZE) - 1;
    if (relative_y >= GRID_SIZE * GRID_SQUARE_SIZE) relative_y = (GRID_SIZE * GRID_SQUARE_SIZE) - 1;

    // Now map to grid cell
    cell.x = relative_x / GRID_SQUARE_SIZE;
    cell.y = relative_y / GRID_SQUARE_SIZE;

    return cell;
}

//////////////////////////////////////////////////////////////////////////////
// ----------------------- SHIP - CLASS  -------------------------------------
//////////////////////////////////////////////////////////////////////////////

const std::unordered_map<SHIP_TYPE, std::pair<std::string, uint8_t>> SHIP_INFO = {
    {SHIP_TYPE::Carrier,    {"Carrier",    5}},
    {SHIP_TYPE::Battleship, {"Battleship", 4}},
    {SHIP_TYPE::Cruiser,    {"Cruiser",    3}},
    {SHIP_TYPE::Submarine,  {"Submarine",  3}},
    {SHIP_TYPE::Destroyer,  {"Destroyer",  2}}
};

SHIP::SHIP(SHIP_TYPE type, SHIP_ORIENTATION orient, Coordinate8 start) {
    uint8_t length = SHIP_INFO.at(type).second;
    cout<<"Start:"<<start<<endl;
    cout<<"length:"<<(int)length<<endl;
    // We want the squares where end coordinates can go to light up
    if(orient == SHIP_ORIENTATION::HORIZONTAL) {
        Coordinate8 pos = start;
        for(uint8_t i=0; i<length; i++) { 
            pos.x =start.x + i;
            drawPegShip(pos.x, start.y);
            positions.push_back(pos);
        }
    }
    else {
        for(uint8_t i=0; i<length; i++) { 
            Coordinate8 pos = start;  
            pos.y = start.y + i;
            drawPegShip(start.x, pos.y);
            positions.push_back(pos);
        }
    }
    hits.resize(length, false); // setting hits vector to same size as length to keep track of hits.
}


bool SHIP::is_sunk() {
    for(bool hit: hits)
    {
        if(!hit) return false; // even if one position is not hit then its not sunk
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////
// ----------------------- GAME - CLASS  -------------------------------------
//////////////////////////////////////////////////////////////////////////////

// Try to place a ship on the board
bool GameBoard::place_ship(SHIP_TYPE type, SHIP_ORIENTATION orientation, Coordinate8 start) {
    uint8_t length = SHIP_INFO.at(type).second;
    vector<Coordinate8> temp_positions;

    // First, check if the ship would go out of bounds
    if (orientation == SHIP_ORIENTATION::HORIZONTAL) {
        if (start.x + length > BOARD_SIZE) {
            return false; // Would go outside board horizontally
        }
    } else { // VERTICAL
        if (start.y + length > BOARD_SIZE) {
            return false; // Would go outside board vertically
        }
    }

    // Now, check if the path is clear (no ships overlapping)
    for (uint8_t i = 0; i < length; i++) {
        Coordinate8 pos = start;
        if (orientation == SHIP_ORIENTATION::HORIZONTAL) {
            pos.x += i;
        } else {
            pos.y += i;
        }

        if (board[pos.x][pos.y] != GRID_STATE::WATER) {
            return false; // Something already here
        }

        temp_positions.push_back(pos);
    }

    // Now we know it's valid, let's place the ship
    SHIP new_ship(type, orientation, start);
    ships.push_back(new_ship);

    for (const auto& pos : new_ship.positions) {
        board[pos.x][pos.y] = GRID_STATE::SHIP;
    }

    return true; // Ship successfully placed
}

// Attack a given target on the board
GRID_STATE GameBoard::attack(Coordinate8& target) {
    if (target.x >= BOARD_SIZE || target.y >= BOARD_SIZE) {
        return GRID_STATE::WATER; // invalid input, treat as miss
    }

    GRID_STATE& cell = board[target.x][target.y];

    if (cell == GRID_STATE::SHIP) {
        // It's a hit!
        cell = GRID_STATE::HIT;
        
        // Update the corresponding ship's hit status
        for (auto& ship : ships) {
            for (size_t i = 0; i < ship.positions.size(); i++) {
                if (ship.positions[i] == target) {
                    ship.hits[i] = true;
                    break;
                }
            }
        }

        // Draw a hit peg
        drawPegHit(target.x, target.y);
        return GRID_STATE::HIT;
    }
    else if (cell == GRID_STATE::WATER) {
        // It's a miss
        cell = GRID_STATE::MISS;
        drawPegMiss(target.x, target.y); // maybe blue for miss?
        return GRID_STATE::MISS;
    }
    else {
        // Already hit or missed → do nothing
        return GRID_STATE::REPEAT;
    }
}

// Check if all ships are sunk (end game)
bool GameBoard::all_ships_sunk() {
    for (auto& ship : ships) {
        if (!ship.is_sunk()) {
            return false;
        }
    }
    return true; // All ships sunk
}