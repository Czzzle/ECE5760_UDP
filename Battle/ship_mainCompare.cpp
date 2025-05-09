//==================================================
// BATTLESHIP MAIN
//==================================================

#include "Battleship_main.h"

using namespace std;
GameBoard playerBoard;

// the color of the boid
char color = WHITE;

// Boid on core 0
fix15 cursorpos_x;
fix15 cursorpos_y;
fix15 cursorpos_cx;
fix15 cursorpos_cy;

// Boid on core 1
fix15 cursorpos_prev_x = 0;
fix15 cursorpos_prev_y = 0;

// =========================== JOYSTICK - CURSOR =================

void spawnCursor(fix15* x, fix15* y, fix15* vx, fix15* vy)
{
  *x = int2fix15(18) ;
  *y = int2fix15(18) ;
  *vx = int2fix15(0);
  *vy = int2fix15(0); 
}

void checkPOS(fix15* x, fix15* y, fix15* vx, fix15* vy)
{
  if (gpio_get(JOY_RIGHT) == 0)
  {
    *vx = int2fix15(-STRIDE_LENGTH);
    *vy = int2fix15(0);  
  }
  else if (gpio_get(JOY_LEFT) == 0)
  {
    *vx = int2fix15(STRIDE_LENGTH);
    *vy = int2fix15(0);
  }
  else if (gpio_get(JOY_UP) == 0)
  {
    *vx = int2fix15(0);
    *vy = int2fix15(STRIDE_LENGTH);
  }
  else if (gpio_get(JOY_DOWN) == 0)
  {
    *vx = int2fix15(0);
    *vy = int2fix15(-STRIDE_LENGTH);
  }
  else
  {
    *vx = int2fix15(0);
    *vy = int2fix15(0);
  }
  *x = *x + *vx ;
  *y = *y + *vy ;

  sleep_ms(200);
}

void wrapCursor(fix15 *x_pos, fix15 *y_pos)
{
  // Wrap around horizontally
  if (*x_pos > int2fix15(SCREEN_WIDTH))
  {
    *x_pos = int2fix15(0);
  }
  else if (*x_pos < int2fix15(0))
  {
    *x_pos = int2fix15(SCREEN_WIDTH);
  }

  // Wrap around vertically
  if (*y_pos > int2fix15(SCREEN_HEIGHT))
  {
    *y_pos = int2fix15(0);
  }
  else if (*y_pos < int2fix15(0))
  {
    *y_pos = int2fix15(SCREEN_HEIGHT);
  }
}

Coordinate8 isInMYGRID(int x_pos, int y_pos) {
  Coordinate8 coord;

  int grid_end_x = LEFT_GRID_X + 10 * GRID_SQUARE_SIZE;
  int grid_end_y = LEFT_GRID_Y + 10 * GRID_SQUARE_SIZE;

  if (x_pos >= LEFT_GRID_X && x_pos < grid_end_x &&
      y_pos >= LEFT_GRID_Y && y_pos < grid_end_y) {
      coord.x = (x_pos - LEFT_GRID_X) / GRID_SQUARE_SIZE;
      coord.y = (y_pos - LEFT_GRID_Y) / GRID_SQUARE_SIZE;
  } else {
      coord.x = 30;
      coord.y = 30;
  }

  return coord;
}

Coordinate8 isInOtherGRID(int x_pos, int y_pos) {
  Coordinate8 coord;

  int grid_end_x = RIGHT_GRID_X + 10 * GRID_SQUARE_SIZE;
  int grid_end_y = RIGHT_GRID_Y + 10 * GRID_SQUARE_SIZE;

  if (x_pos >= RIGHT_GRID_X && x_pos < grid_end_x &&
      y_pos >= RIGHT_GRID_Y && y_pos < grid_end_y) {
      coord.x = (x_pos - RIGHT_GRID_X) / GRID_SQUARE_SIZE - 26;
      coord.y = (y_pos - RIGHT_GRID_Y) / GRID_SQUARE_SIZE;
  } else {
      coord.x = 30;
      coord.y = 30;
  }

  return coord;
}


// ==================================================
// === users serial input thread
// ==================================================

// static PT_THREAD(protothread_serial(struct pt *pt))
// {
//   PT_BEGIN(pt);
//     // static int user_input;
//     static char coord_input[4];
//     static Coordinate8 attack_coord;
//     PT_YIELD_usec(1000000);
//     while(1)
//     {    
//       // wait for 0.1 sec
//       if(playerBoard.game_status==GAME_STATUS::ONGOING) {
//           // print prompt
//           // printf("\nEntered Serial loop");
//           sprintf(pt_serial_out_buffer, "Battleship test mode: enter attacks like A5 or B3\n\r");
//           serial_write;
//           sprintf(pt_serial_out_buffer, "Enter attack coordinate (e.g., A5): ");
//           serial_write;
//           serial_read;
//           sscanf(pt_serial_in_buffer, "%s", coord_input);
//           attack_coord = decodeCoord(coord_input);
//           GRID_STATE result = playerBoard.attack(attack_coord);
//       } 
//     } // END WHILE(1)
//   PT_END(pt);
// } // timer thread

// ==================================================
// === Animation Thread
// ==================================================

// Animation on core 0
static PT_THREAD(protothread_anim(struct pt *pt))
{
  // Mark beginning of thread
  PT_BEGIN(pt);

    static int begin_time;
    static int spare_time;
  
    static char buffer1[50];    
    
    spawnCursor(&cursorpos_x, &cursorpos_y, &cursorpos_cx, &cursorpos_cy); // Spawn a boid
    // drawCursor(fix2int15(cursorpos_x), fix2int15(cursorpos_y), color);
    moveCursor(&cursorpos_prev_x, &cursorpos_prev_y, cursorpos_x, cursorpos_y, color);
    // savePrevPixels(fix2int15(cursorpos_x), fix2int15(cursorpos_y));
    // cursorpos_prev_x = cursorpos_x;
    // cursorpos_prev_y = cursorpos_y;

    uint8_t prev_val = 0;
    uint8_t val_ship = 0;
    bool select_flag = false;
    uint8_t ctr_ship = 0;
    int ctr_button =0;
    static bool prev_left_button_state;
    bool curr_left_button_state;
    static bool prev_right_button_state;
    bool curr_right_button_state;

    while(1) {
      // Measure time at start of thread
      begin_time = time_us_32() ; 
      //LED on
      // gpio_put(LED_PIN, !gpio_get(LED_PIN)); 
      checkPOS(&cursorpos_x, &cursorpos_y, &cursorpos_cx, &cursorpos_cy);
      // printf("\nGAME_STATUS: %d", playerBoard.game_status_check());
      wrapCursor(&cursorpos_x, &cursorpos_y);
      // If cursor moved
      // if(gpio_get(BUT_PIN_Y)==1)
      // {
      //   printf("\nButton Pressed:%d", ++ctr_button);
      // }
      prev_left_button_state = true;  // pulled-up, so default is HIGH
      prev_right_button_state = true;
      curr_left_button_state = gpio_get(BUT_PIN_Y);
      curr_right_button_state = gpio_get(BUT_PIN_R);
      // printf("prev=%x && current=%x\n",prev_left_button_state, curr_left_button_state);
      if (cursorpos_prev_x != cursorpos_x | cursorpos_prev_y != cursorpos_y) {
        void drawBoundary();
        printf("\nOpponentGame Status %d", oponent_player);
        
        fillRect(10, SCREEN_HEIGHT-10, 100, 10, BLACK);
        sprintf(buffer1, "x: %d, y: %d", fix2int15(cursorpos_x), fix2int15(cursorpos_y));
        setCursor(10, SCREEN_HEIGHT-10);
        setTextColor(WHITE);
        setTextSize(1);
        writeString(buffer1);

        // drawCursor(fix2int15(cursorpos_prev_x), fix2int15(cursorpos_prev_y), BLACK);
        // cursorpos_prev_x = cursorpos_x;
        // cursorpos_prev_y = cursorpos_y;
        // moveCursor(&cursorpos_prev_x, &cursorpos_prev_y, cursorpos_x, cursorpos_y, color);

        int intcursor_x = fix2int15(cursorpos_x);
        int intcursor_y = fix2int15(cursorpos_y);

        if(playerBoard.game_status==GAME_STATUS::INITIAL)
        {
          if(checkCursorOverStartButton(intcursor_x, intcursor_y))
          {  
            printf("\nReached here");
            if(prev_left_button_state == curr_left_button_state)
            { 
              printf("\nButtonloop");
              welcomeText(BLACK);
              playerBoard.game_status = GAME_STATUS::LEVEL;
              difficultyChoose(YELLOW, RED, 0, 1);
              printf("\nGAME_STATUS: %d", playerBoard.game_status_check());
            }
          }
        }
        else if(playerBoard.game_status==GAME_STATUS::LEVEL)
        {
          uint8_t val = checkCursorOverLevel(intcursor_x, intcursor_y);
          if(prev_left_button_state == curr_left_button_state)
          {
            if(val!=0)
            {
              difficultyChoose(YELLOW, BLUE, 0, 0);
              playerBoard.game_status = GAME_STATUS::PLACE;
              // spawnCursor(&cursorpos_x, &cursorpos_y, &cursorpos_cx, &cursorpos_cy);
              drawGRID(BOARD_SIZE, LEFT_GRID_X, LEFT_GRID_Y, GRID_OUTLINE, BLUE);
              drawGridDim(LEFT_GRID_X, LEFT_GRID_Y, WHITE);
              drawTextforShip(YELLOW, BLUE, 1);
              printf("\nGAME_STATUS: %d", playerBoard.game_status_check());
            }
          }
        }
        else if(playerBoard.game_status==GAME_STATUS::PLACE)
        {
          const char* shipname;
          if(ctr_ship!=5)
          {
            if(select_flag == false)
            {
              val_ship = checkCursorOverShip(intcursor_x, intcursor_y);
              if(val_ship!=0 && prev_val!=val_ship && prev_left_button_state==curr_left_button_state)
              {
                if(val_ship==1)
                {
                  shipname = "Carrier";
                  drawBoxforShip(RED, RIGHT_GRID_X, SHIPLIST_SPACE_Carrier, shipname);
                  prev_val = val_ship;
                  select_flag = true;
                  // spawnCursor(&cursorpos_x, &cursorpos_y, &cursorpos_cx, &cursorpos_cy);
                }
                else if(val_ship==2)
                {
                  shipname = "Batteship";
                  drawBoxforShip(RED, RIGHT_GRID_X, SHIPLIST_SPACE_Battleship, shipname);
                  prev_val = val_ship;
                  select_flag = true;
                  // spawnCursor(&cursorpos_x, &cursorpos_y, &cursorpos_cx, &cursorpos_cy);              
                }
                else if(val_ship==3)
                {
                  shipname = "Cruiser";
                  drawBoxforShip(RED, RIGHT_GRID_X, SHIPLIST_SPACE_Cruiser, shipname);
                  prev_val = val_ship;
                  select_flag = true;
                  // spawnCursor(&cursorpos_x, &cursorpos_y, &cursorpos_cx, &cursorpos_cy);
                }
                else if(val_ship==4)
                {
                  shipname = "Submarine";
                  drawBoxforShip(RED, RIGHT_GRID_X, SHIPLIST_SPACE_Submarine, shipname);
                  prev_val = val_ship;
                  select_flag = true;
                  // spawnCursor(&cursorpos_x, &cursorpos_y, &cursorpos_cx, &cursorpos_cy);
                }
                else if(val_ship==5)
                {
                  shipname = "Destroyer";
                  drawBoxforShip(RED, RIGHT_GRID_X, SHIPLIST_SPACE_Destroyer, shipname);
                  prev_val = val_ship;
                  select_flag = true;
                  // spawnCursor(&cursorpos_x, &cursorpos_y, &cursorpos_cx, &cursorpos_cy);
                }
              }
            }
            else if(select_flag)
            {
              if(val_ship!=0)
              {
                Coordinate8 grid_pos = isInMYGRID(intcursor_x, intcursor_y);
                printf("prev:%x, curr:%x",prev_right_button_state,curr_right_button_state);

                // Check if we're within the grid (i.e., not the sentinel value 30,30)
                if (prev_left_button_state==curr_left_button_state && !(grid_pos.x == 30 && grid_pos.y == 30))
                {
                  char encoded[4];  // Enough space for something like "A10" + null terminator
                  encodeCoord(grid_pos, encoded);
                  printf("\nEncoded:%c %c, VAL_SHIP:%d", encoded[0],encoded[1], val_ship-1);
                  bool success = playerBoard.place_ship(SHIP_TYPE(val_ship-1), SHIP_ORIENTATION::HORIZONTAL, grid_pos);
                          
                  if (success) {
                    ctr_ship++;                  // Increase placed ship counter
                    select_flag = false;        // Reset flag to allow next ship to be selected
                    printf("Ship placed successfully at %d,%d\n", grid_pos.x, grid_pos.y);
                  } 
                  else  printf("Invalid position. Try again.\n");               
                }
                // Check if we're within the grid (i.e., not the sentinel value 30,30)
                else if (prev_right_button_state==curr_right_button_state && !(grid_pos.x == 30 && grid_pos.y == 30))
                {
                  char encoded[4];  // Enough space for something like "A10" + null terminator
                  encodeCoord(grid_pos, encoded);
                  printf("\nEncoded:%c %c, VAL_SHIP:%d", encoded[0],encoded[1], val_ship-1);
                  bool success = playerBoard.place_ship(SHIP_TYPE(val_ship-1), SHIP_ORIENTATION::VERTICAL, grid_pos);
                          
                  if (success) {
                    ctr_ship++;                  // Increase placed ship counter
                    select_flag = false;        // Reset flag to allow next ship to be selected
                    printf("Ship placed successfully at %d,%d\n", grid_pos.x, grid_pos.y);
                  } 
                  else  printf("Invalid position. Try again.\n");               
                }
              }
            }
          }
          else 
          {
            printf("\nReached here finish placement");
            if(checkCursorOverStartGame(intcursor_x, intcursor_y) && prev_left_button_state==curr_left_button_state)
            {
              playerBoard.game_status=GAME_STATUS::ONGOING;
              printf("\nGAME_STATUS: %d", playerBoard.game_status_check());
              drawTextforShip(BLACK,BLACK,1);
              drawBlackBoxforShip();
              drawGRID(BOARD_SIZE, RIGHT_GRID_X, RIGHT_GRID_Y, GRID_OUTLINE, BLUE);
              drawGridDim(RIGHT_GRID_X, RIGHT_GRID_Y, WHITE);
              
            }
          }
        }
        else if(playerBoard.game_status==GAME_STATUS::ONGOING)
        {
          // printf("\nOpponentGame Status %d", oponent_player);
          printf("\nTURN:%x, RESPONSE:%x, ATTACK:%x", your_turn, received_response, received_attack);
          Coordinate8 posn;
          posn.x = uint8_t(intcursor_x);
          posn.y = uint8_t(intcursor_y);
          if(oponent_player==GAME_STATUS::ONGOING)
          {
            Coordinate8 grid_pos = isInOtherGRID(intcursor_x, intcursor_y);
            printf("INT GRID X=%d, GRID Y=%d",(int)grid_pos.x, (int)grid_pos.y);
            // printf("UINT GRID X=%d, GRID Y=%d",grid_pos.x, grid_pos.y);
            // PRINT HERE TURN RESULT ALSO
            if(your_turn) { 
              if(!(grid_pos.x == 30 && grid_pos.y == 30) && prev_left_button_state==curr_left_button_state)
              {
                drawPegPotentialShip(grid_pos.x, grid_pos.y);
                printf("\nEntered send attack segment");
                raw_send(GAME_STATUS::ONGOING, GRID_STATE::REPEAT, grid_pos, 3);
                your_turn = false;
                received_response = true;
              }
            }
            else if(received_response)
            {
              if(opponent_gridstate==GRID_STATE::HIT)
                drawPegHit((int)grid_pos.x, (int)grid_pos.y);
              else if(opponent_gridstate==GRID_STATE::MISS)
                drawPegMiss((int)grid_pos.x, (int)grid_pos.y);
                received_response = false;
                received_attack = true;
              // else if(opponent_gridstate==GRID_STATE::REPEAT)
            }
            else if(received_attack) {
                GRID_STATE my_GRID = playerBoard.attack(our_shippos);
                if(playerBoard.all_ships_sunk())
                  raw_send(GAME_STATUS::WIN, GRID_STATE::HIT,posn, 1);
                else
                {
                  raw_send(GAME_STATUS::ONGOING, my_GRID, posn, 2);
                }
                received_attack = false;
                your_turn = true;
            }
          }
          else {
            raw_send(GAME_STATUS::ONGOING, GRID_STATE::WATER, posn, 1);
          }
          // (GAME_STATUS status, GRID_STATE state, Coordinate8 coord, int sendOption)
        }
        moveCursor(&cursorpos_prev_x, &cursorpos_prev_y, cursorpos_x, cursorpos_y, color);

      } // End if cursor moved
      
      drawCursor(fix2int15(cursorpos_x), fix2int15(cursorpos_y), color); // draw the boid at its new position
      
      spare_time = FRAME_RATE - (time_us_32() - begin_time) ;   
    } // END WHILE(1)
  PT_END(pt);

} // animation thread

// ==================================================
// === CORE 1 MAIN 
// ==================================================
void core1_main()
{
  initVGA();
  welcomeText(YELLOW);
  raw_send_test();
  pt_add_thread(protothread_anim);
  // pt_add_thread(protothread_serial);
  pt_schedule_start;
}

// ========================================
// === main
// ========================================

int main()
{
  stdio_init_all();
  printf("Start main\n");

  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);

  //initialize button
  gpio_init(BUT_PIN_Y);
  gpio_set_dir(BUT_PIN_Y, GPIO_IN);
  gpio_pull_up(BUT_PIN_Y);

  gpio_init(BUT_PIN_R);
  gpio_set_dir(BUT_PIN_R, GPIO_IN);
  gpio_pull_up(BUT_PIN_R);
  
  //initialize Joystick
  // set up gpio 4 for joystick button	
  gpio_init(JOY_RIGHT); //right 
  gpio_init(JOY_LEFT); //left
  gpio_init(JOY_UP); //up
  gpio_init(JOY_DOWN); //down

  gpio_set_dir(JOY_RIGHT, GPIO_IN) ;
  gpio_set_dir(JOY_LEFT, GPIO_IN) ;
  gpio_set_dir(JOY_UP, GPIO_IN) ;
  gpio_set_dir(JOY_DOWN, GPIO_IN) ;

  // pullup ON, pulldown OFF
  gpio_pull_up (JOY_RIGHT);
  gpio_pull_up (JOY_LEFT);
  gpio_pull_up (JOY_UP);
  gpio_pull_up (JOY_DOWN);

  // playerBoard.game_status = GAME_STATUS::PLACE;
  printf("\nGAME_STATUS: %d", playerBoard.game_status_check());

  memset(received_data, 0, BEACON_MSG_LEN_MAX); // clean received_data content (remove garbge)
  // Connect to WiFi
  if (connectWifi(country, WIFI_SSID, WIFI_PASSWORD, auth))
  {
    printf("Failed connection.\n");
  }
  else
  {

    printf("My IP is: %s\n", ip4addr_ntoa(netif_ip_addr4(netif_default)));
  }

  // Initialize semaphore
  PT_SEM_INIT(&new_message, 0);
  PT_SEM_INIT(&ready_to_send, 0);

  //============================
  // UDP recenve ISR routines
  udpecho_raw_init();
  
  multicore_reset_core1();
  multicore_launch_core1(&core1_main);

  // sleep_ms(100);
  pt_add_thread(protothread_send);
  pt_add_thread(protothread_receive);

  // start scheduler
  pt_schedule_start;
}