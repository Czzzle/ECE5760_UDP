#include <iostream>
#include <stdio.h>
#include <stdlib.h>     // for rand()
#include <time.h>       // for seeding rand()

#include "pico/stdlib.h"

#include "BattleshipGame.h"
#include "vga_displayElements.h"


using namespace std;

Coordinatefix15 CursorPosition = {30, 24};

int main() {
    stdio_init_all();
    initVGA();
    drawGRID(BOARD_SIZE, LEFT_GRID_X, LEFT_GRID_Y, GRID_OUTLINE, YELLOW);

    GameBoard playerBoard;

    srand(time(NULL)); // Random seed

    printf("\n=== Welcome to Battleship VGA Simulation ===\n");

    // Step 1: Place Ships Automatically
    for (int i = 0; i < 5; ++i) { // Place 5 ships
        bool placed = false;
        while (!placed) {
            Coordinate8 start;
            start.x = rand() % BOARD_SIZE;
            start.y = rand() % BOARD_SIZE;

            SHIP_ORIENTATION orientation = (rand() % 2 == 0) ? SHIP_ORIENTATION::HORIZONTAL : SHIP_ORIENTATION::VERTICAL;
            SHIP_TYPE type = static_cast<SHIP_TYPE>(i % 5); // Cycle through types

            placed = playerBoard.place_ship(type, orientation, start);
        }
    }
    printf("Ships placed!\n");

    sleep_ms(1000); // Give some time to view the grid first

    // Step 2: Simulate Random Attacks
    for (int turn = 0; turn < BOARD_SIZE * 2; ++turn) { // At most BOARD_SIZE^2 turns
        Coordinate8 attackCoord;
        attackCoord.x = rand() % BOARD_SIZE;
        attackCoord.y = rand() % BOARD_SIZE;

        GRID_STATE result = playerBoard.attack(attackCoord);

        if (result == GRID_STATE::HIT) {
            printf("Hit at %c%d\n", 'A' + attackCoord.x, attackCoord.y + 1);
            drawPegHit(attackCoord.x, attackCoord.y); // Draw red
        }
        else if (result == GRID_STATE::MISS) {
            printf("Miss at %c%d\n", 'A' + attackCoord.x, attackCoord.y + 1);
            drawPegMiss(attackCoord.x, attackCoord.y); // Draw white
        }
        else {
            // Already attacked, skip printing
            continue;
        }

        sleep_ms(500); // Slow down so you can see each attack

        // Step 3: Check if game is over
        if (playerBoard.all_ships_sunk()) {
            printf("\nAll ships sunk! Game over.\n");
            break;
        }
    }

    printf("\nSimulation complete.\n");

    return 0;

}