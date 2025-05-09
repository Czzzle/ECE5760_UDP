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

enum PlayerState
{
  YT,
  RR,
  RA
}; // Your Turn, Ready Response, Ready Attack
PlayerState my_state = RA;
PlayerState opponent_state = YT;


// =================== declare DMA chains =========
int data_chan_boom;
int ctrl_chan_boom;
int data_chan_splash;
int ctrl_chan_splash;

// =========================== JOYSTICK - CURSOR =================

void spawnCursor(fix15 *x, fix15 *y, fix15 *vx, fix15 *vy)
{
  *x = int2fix15(77);
  *y = int2fix15(79);
  *vx = int2fix15(0);
  *vy = int2fix15(0);
}

void checkPOS(fix15 *x, fix15 *y, fix15 *vx, fix15 *vy)
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
  *x = *x + *vx;
  *y = *y + *vy;

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

Coordinate8 isInMYGRID(int x_pos, int y_pos)
{
  Coordinate8 coord;

  int grid_end_x = LEFT_GRID_X + 10 * GRID_SQUARE_SIZE;
  int grid_end_y = LEFT_GRID_Y + 10 * GRID_SQUARE_SIZE;

  if (x_pos >= LEFT_GRID_X && x_pos < grid_end_x &&
      y_pos >= LEFT_GRID_Y && y_pos < grid_end_y)
  {
    coord.x = (x_pos - LEFT_GRID_X) / GRID_SQUARE_SIZE;
    coord.y = (y_pos - LEFT_GRID_Y) / GRID_SQUARE_SIZE;
  }
  else
  {
    coord.x = 30;
    coord.y = 30;
  }

  return coord;
}

Coordinate8 isInOtherGRID(int x_pos, int y_pos)
{
  Coordinate8 coord;

  int grid_end_x = RIGHT_GRID_X + 10 * GRID_SQUARE_SIZE;
  int grid_end_y = RIGHT_GRID_Y + 10 * GRID_SQUARE_SIZE;

  if (x_pos >= RIGHT_GRID_X && x_pos < grid_end_x &&
      y_pos >= RIGHT_GRID_Y && y_pos < grid_end_y)
  {
    // coord.x = (x_pos - RIGHT_GRID_X) / GRID_SQUARE_SIZE; //Chengle's version
    coord.x = (x_pos - RIGHT_GRID_X) / GRID_SQUARE_SIZE - 26; // diksh's version
    coord.y = (y_pos - RIGHT_GRID_Y) / GRID_SQUARE_SIZE;
  }
  else
  {
    coord.x = 30;
    coord.y = 30;
  }

  return coord;
}

//=========================== IRQ - Button ===================
volatile bool prev_yellow_button_state = false;
volatile bool prev_red_button_state = false;
volatile bool yellow_button_state = false; // new variable with similar name!!!!
volatile bool red_button_state = false;
volatile bool yellow_pressed = false; // button change from 0 to 1 --> pressed = 1
volatile bool red_pressed = false;

static void button_irq(void)
{
  // Assert a GPIO when we enter the interrupt
  gpio_put(ISR_GPIO, 1);

  // Clear the alarm irq
  hw_clear_bits(&timer_hw->intr, 1u << ALARM_NUM);

  // Reset the alarm register
  timer_hw->alarm[ALARM_NUM] = timer_hw->timerawl + DELAY;

  // ------ MAIN FUNCTION HERE ----------
  // update previous state
  prev_yellow_button_state = yellow_button_state;
  prev_red_button_state = red_button_state;

  // set current state
  yellow_button_state = gpio_get(BUT_PIN_Y);
  red_button_state = gpio_get(BUT_PIN_R);

  // by default, we assume no button change from 0 to 1
  yellow_pressed = false;
  red_pressed = false;

  // onlt 0->1 is press
  if (prev_yellow_button_state != yellow_button_state)
  {
    // printf("Yellow: from %b to %b", yellow_button_state, y_state);
    if (yellow_button_state == 1)
    {
      yellow_pressed = true;
      printf("\nyellow pressed");
    }
  }

  if (prev_red_button_state != red_button_state)
  {
    // printf("Red: from %b to %b", red_button_state, r_state);
    if (red_button_state == 1)
    {
      red_pressed = true;
      printf("\nred pressed");
    }
  }

  // ------------------------------------
  // De-assert the GPIO when we leave the interrupt
  gpio_put(ISR_GPIO, 0);
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
int intcursor_x;
int intcursor_y;

static PT_THREAD(protothread_anim(struct pt *pt))
{
  // Mark beginning of thread
  PT_BEGIN(pt);

  static int begin_time;
  static int spare_time;

  static char buffer1[50];

  // spawnCursor(&cursorpos_x, &cursorpos_y, &cursorpos_cx, &cursorpos_cy); // Spawn a boid at (20,20,0,0)
  // moveCursor(&cursorpos_prev_x, &cursorpos_prev_y, cursorpos_x, cursorpos_y, color);

  uint8_t prev_val = 0;
  uint8_t val_ship = 0;
  bool select_flag = false;
  uint8_t ctr_ship = 0;
  int ctr_button = 0;
  // static bool prev_left_button_state = false;
  // bool curr_left_button_state = false;
  // static bool prev_right_button_state = false;
  // bool curr_right_button_state = false;

  // PT_SEM_SAFE_WAIT(pt, &new_message);

  while (1)
  {
    // Measure time at start of thread
    begin_time = time_us_32();

    // Each Page Working
    if (playerBoard.game_status == GAME_STATUS::INITIAL)
    {
      if (yellow_pressed)
      {
        welcomeText(BLACK);

        playerBoard.game_status = GAME_STATUS::LEVEL;

        raw_send(GAME_STATUS::LEVEL, GRID_STATE::WATER, {(uint8_t)intcursor_x, (uint8_t)intcursor_y}, 1);

        difficultyChoose(YELLOW, RED, 0, 1);
        printf("\nGAME_STATUS: %d", playerBoard.game_status_check());

        yellow_pressed = false; // avoid skip difficulty level
      }
    }
    else if (playerBoard.game_status == GAME_STATUS::LEVEL)
    {

      // "Easy" == Yellow button
      if (yellow_pressed)
      {
        difficultyChoose(YELLOW, BLUE, 0, 0);
        playerBoard.game_status = GAME_STATUS::PLACE;

        drawGRID(BOARD_SIZE, LEFT_GRID_X, LEFT_GRID_Y, GRID_OUTLINE, BLUE);
        drawGridDim(LEFT_GRID_X, LEFT_GRID_Y, WHITE);
        drawTextforShip(YELLOW, BLUE, 1);
        printf("\nGAME_STATUS: %d", playerBoard.game_status_check());

        spawnCursor(&cursorpos_x, &cursorpos_y, &cursorpos_cx, &cursorpos_cy); // Spawn a boid
        // drawCursor(fix2int15(cursorpos_x), fix2int15(cursorpos_y), color);
        moveCursor(&cursorpos_prev_x, &cursorpos_prev_y, cursorpos_x, cursorpos_y, color);

        yellow_pressed = false;
      }

      //"Hard" == Red button
      if (red_pressed)
      {
        difficultyChoose(YELLOW, BLUE, 0, 0);
        playerBoard.game_status = GAME_STATUS::PLACE;

        drawGRID(BOARD_SIZE, LEFT_GRID_X, LEFT_GRID_Y, GRID_OUTLINE, BLUE);
        drawGridDim(LEFT_GRID_X, LEFT_GRID_Y, WHITE);
        drawTextforShip(YELLOW, BLUE, 1);
        printf("\nGAME_STATUS: %d", playerBoard.game_status_check());

        spawnCursor(&cursorpos_x, &cursorpos_y, &cursorpos_cx, &cursorpos_cy); // Spawn a boid
        // drawCursor(fix2int15(cursorpos_x), fix2int15(cursorpos_y), color);
        moveCursor(&cursorpos_prev_x, &cursorpos_prev_y, cursorpos_x, cursorpos_y, color);

        red_pressed = false;
      }
    }
    else if (playerBoard.game_status == GAME_STATUS::PLACE)
    {
      checkPOS(&cursorpos_x, &cursorpos_y, &cursorpos_cx, &cursorpos_cy);
      wrapCursor(&cursorpos_x, &cursorpos_y);

      if (cursorpos_prev_x != cursorpos_x | cursorpos_prev_y != cursorpos_y)
      {
        void drawBoundary();
        printf("\nOpponentGame Status %d", opponent_player);

        fillRect(10, SCREEN_HEIGHT - 10, 100, 10, BLACK);
        sprintf(buffer1, "x: %d, y: %d", fix2int15(cursorpos_x), fix2int15(cursorpos_y));
        setCursor(10, SCREEN_HEIGHT - 10);
        setTextColor(WHITE);
        setTextSize(1);
        writeString(buffer1);

        // drawCursor(fix2int15(cursorpos_prev_x), fix2int15(cursorpos_prev_y), BLACK);
        // cursorpos_prev_x = cursorpos_x;
        // cursorpos_prev_y = cursorpos_y;
        moveCursor(&cursorpos_prev_x, &cursorpos_prev_y, cursorpos_x, cursorpos_y, color);
      }

      intcursor_x = fix2int15(cursorpos_x);
      intcursor_y = fix2int15(cursorpos_y);

      const char *shipname;
      if (ctr_ship != 5)
      {

        if (select_flag == false)
        {
          val_ship = checkCursorOverShip(intcursor_x, intcursor_y);
          // printf("val_ship = %d\n", val_ship);
          // printf("prev_val = %d\n", prev_val);
          if (val_ship != 0 && prev_val != val_ship && yellow_button_state && prev_yellow_button_state)
          {
            printf("pass through\n");
            if (val_ship == 1)
            {
              shipname = "Carrier    (5)";
              drawBoxforShip(RED, RIGHT_GRID_X, SHIPLIST_SPACE_Carrier, shipname);

              moveCursor(&cursorpos_x, &cursorpos_y, cursorpos_x, cursorpos_y, color);

              prev_val = val_ship;
              select_flag = true;
              // spawnCursor(&cursorpos_x, &cursorpos_y, &cursorpos_cx, &cursorpos_cy);
            }
            else if (val_ship == 2)
            {
              printf("select batteship");
              shipname = "Battleship (4)";
              drawBoxforShip(RED, RIGHT_GRID_X, SHIPLIST_SPACE_Battleship, shipname);

              moveCursor(&cursorpos_x, &cursorpos_y, cursorpos_x, cursorpos_y, color);

              prev_val = val_ship;
              select_flag = true;
              // spawnCursor(&cursorpos_x, &cursorpos_y, &cursorpos_cx, &cursorpos_cy);
            }
            else if (val_ship == 3)
            {
              shipname = "Cruiser    (3)";
              drawBoxforShip(RED, RIGHT_GRID_X, SHIPLIST_SPACE_Cruiser, shipname);

              moveCursor(&cursorpos_x, &cursorpos_y, cursorpos_x, cursorpos_y, color);

              prev_val = val_ship;
              select_flag = true;
              // spawnCursor(&cursorpos_x, &cursorpos_y, &cursorpos_cx, &cursorpos_cy);
            }
            else if (val_ship == 4)
            {
              shipname = "Submarine  (3)";
              drawBoxforShip(RED, RIGHT_GRID_X, SHIPLIST_SPACE_Submarine, shipname);

              moveCursor(&cursorpos_x, &cursorpos_y, cursorpos_x, cursorpos_y, color);

              prev_val = val_ship;
              select_flag = true;
              // spawnCursor(&cursorpos_x, &cursorpos_y, &cursorpos_cx, &cursorpos_cy);
            }
            else if (val_ship == 5)
            {
              shipname = "Destroyer  (2)";
              drawBoxforShip(RED, RIGHT_GRID_X, SHIPLIST_SPACE_Destroyer, shipname);

              moveCursor(&cursorpos_x, &cursorpos_y, cursorpos_x, cursorpos_y, color);

              prev_val = val_ship;
              select_flag = true;
              // spawnCursor(&cursorpos_x, &cursorpos_y, &cursorpos_cx, &cursorpos_cy);
            }
          }
        }
        else if (select_flag)
        {
          if (val_ship != 0)
          {
            Coordinate8 grid_pos = isInMYGRID(intcursor_x, intcursor_y);
            // printf("prev:%x, curr:%x", prev_right_button_state, curr_right_button_state);

            // Check if we're within the grid (i.e., not the sentinel value 30,30)
            if (yellow_button_state && prev_yellow_button_state && !(grid_pos.x == 30 && grid_pos.y == 30))
            {
              char encoded[4]; // Enough space for something like "A10" + null terminator
              encodeCoord(grid_pos, encoded);
              printf("\nEncoded:%c %c, VAL_SHIP:%d", encoded[0], encoded[1], val_ship - 1);
              bool success = playerBoard.place_ship(SHIP_TYPE(val_ship - 1), SHIP_ORIENTATION::HORIZONTAL, grid_pos);

              if (success)
              {
                moveCursor(&cursorpos_x, &cursorpos_y, cursorpos_x, cursorpos_y, color);

                ctr_ship++;          // Increase placed ship counter
                select_flag = false; // Reset flag to allow next ship to be selected
                printf("Ship placed successfully at %d,%d\n", grid_pos.x, grid_pos.y);
              }
              else
                printf("Invalid position. Try again.\n");
            }
            // Check if we're within the grid (i.e., not the sentinel value 30,30)
            else if (red_button_state && prev_red_button_state && !(grid_pos.x == 30 && grid_pos.y == 30))
            {
              char encoded[4]; // Enough space for something like "A10" + null terminator
              encodeCoord(grid_pos, encoded);
              printf("\nEncoded:%c %c, VAL_SHIP:%d", encoded[0], encoded[1], val_ship - 1);
              bool success = playerBoard.place_ship(SHIP_TYPE(val_ship - 1), SHIP_ORIENTATION::VERTICAL, grid_pos);

              if (success)
              {

                moveCursor(&cursorpos_x, &cursorpos_y, cursorpos_x, cursorpos_y, color);

                ctr_ship++;          // Increase placed ship counter
                select_flag = false; // Reset flag to allow next ship to be selected
                printf("Ship placed successfully at %d,%d\n", grid_pos.x, grid_pos.y);
              }
              else
                printf("Invalid position. Try again.\n");
            }
          }
        }
      }
      else
      {
        // printf("\nReached here finish placement");
        if (checkCursorOverStartGame(intcursor_x, intcursor_y) && yellow_button_state && prev_yellow_button_state)
        {
          playerBoard.game_status = GAME_STATUS::ONGOING;
          raw_send(GAME_STATUS::ONGOING, GRID_STATE::WATER, {(uint8_t)intcursor_x, (uint8_t)intcursor_y}, 1);
          printf("\nGAME_STATUS: %d", playerBoard.game_status_check());
          drawTextforShip(BLACK, BLACK, 1);
          drawBoxforStartGame(BLACK);

          moveCursor(&cursorpos_x, &cursorpos_y, cursorpos_x, cursorpos_y, color);

          drawBlackBoxforShip();
          drawGRID(BOARD_SIZE, RIGHT_GRID_X, RIGHT_GRID_Y, GRID_OUTLINE, BLUE);
          drawGridDim(RIGHT_GRID_X, RIGHT_GRID_Y, WHITE);
        }
      }
    }
    else if (playerBoard.game_status == GAME_STATUS::ONGOING)
    {
      // ------------- Check Cursor Movement --------------
      checkPOS(&cursorpos_x, &cursorpos_y, &cursorpos_cx, &cursorpos_cy); // update cursor loctaion
      wrapCursor(&cursorpos_x, &cursorpos_y);                             // avoid go out of boundary

      if (cursorpos_prev_x != cursorpos_x | cursorpos_prev_y != cursorpos_y)
      {
        // void drawBoundary();
        printf("\nOpponentGame Status %d", opponent_player);

        fillRect(10, SCREEN_HEIGHT - 10, 100, 10, BLACK);
        sprintf(buffer1, "x: %d, y: %d", fix2int15(cursorpos_x), fix2int15(cursorpos_y));
        setCursor(10, SCREEN_HEIGHT - 10);
        setTextColor(WHITE);
        setTextSize(1);
        writeString(buffer1);

        moveCursor(&cursorpos_prev_x, &cursorpos_prev_y, cursorpos_x, cursorpos_y, color);

      } // End if cursor moved

      intcursor_x = fix2int15(cursorpos_x);
      intcursor_y = fix2int15(cursorpos_y);

      // printf("\nOpponentGame Status %d", opponent_player);
      // printf("\nMY_STATE%x", my_state);
      Coordinate8 posn;
      posn.x = uint8_t(intcursor_x);
      posn.y = uint8_t(intcursor_y);

      if (opponent_player == GAME_STATUS::ONGOING)
      {
        Coordinate8 grid_pos = isInOtherGRID(intcursor_x, intcursor_y);
        // PRINT HERE TURN RESULT ALSO
        if (my_state == YT)
        {
          if (!(grid_pos.x == 30 && grid_pos.y == 30) && prev_yellow_button_state && yellow_button_state)
          {
            drawPegPotentialShip(grid_pos.x, grid_pos.y);
            moveCursor(&cursorpos_x, &cursorpos_y, cursorpos_x, cursorpos_y, color);
            printf("\nEntered send attack segment");
            raw_send(GAME_STATUS::ONGOING, GRID_STATE::REPEAT, grid_pos, 3);
            my_state = RR;
            opponent_state = RA;
          }
        }
        else if (my_state == RR && strstr(received_data, "GRID") != NULL)
        {
          if (opponent_gridstate == GRID_STATE::HIT)
          {
            dma_start_channel_mask(1u << ctrl_chan_boom);
            drawPegHitRight((int)grid_pos.x, (int)grid_pos.y);
            moveCursor(&cursorpos_x, &cursorpos_y, cursorpos_x, cursorpos_y, color);
          }

          else if (opponent_gridstate == GRID_STATE::MISS)
          {
            dma_start_channel_mask(1u << ctrl_chan_splash);
            drawPegMissRight((int)grid_pos.x, (int)grid_pos.y);
            moveCursor(&cursorpos_x, &cursorpos_y, cursorpos_x, cursorpos_y, color);
          }

          my_state = RA;
          opponent_state = YT;
        }
        else if (my_state == RA)
        {
          int length_string = strlen(received_data);
          if (length_string <= 18 && length_string > 16)
          {
            GRID_STATE my_GRID = playerBoard.attack(our_shippos);
            if (playerBoard.all_ships_sunk())
            {
              raw_send(GAME_STATUS::WIN, GRID_STATE::HIT, posn, 1);
              playerBoard.game_status = GAME_STATUS::LOSE;
            }
            else
            {
              raw_send(GAME_STATUS::ONGOING, my_GRID, posn, 2);
              my_state = YT;
              opponent_state = RA;
            }
          }
        }
      }
      else
      {
        raw_send(GAME_STATUS::ONGOING, GRID_STATE::WATER, posn, 1);
      }
    }
    else if (opponent_player == GAME_STATUS::LOSE)
    {
      playerBoard.game_status = GAME_STATUS::WIN;
      fillRect(0, 0, 320, 240, BLACK);
      winnerDeclare(YELLOW);
    }
    else if (playerBoard.game_status == GAME_STATUS::LOSE)
    {
      raw_send(GAME_STATUS::LOSE, GRID_STATE::HIT, {30, 30}, 1);
      fillRect(0, 0, 320, 240, BLACK);
      loserDeclare(WHITE);
    }

    spare_time = FRAME_RATE - (time_us_32() - begin_time);
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
  // raw_send_test();
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

  // initialize button
  gpio_init(BUT_PIN_Y);
  gpio_set_dir(BUT_PIN_Y, GPIO_IN);
  gpio_pull_up(BUT_PIN_Y);

  gpio_init(BUT_PIN_R);
  gpio_set_dir(BUT_PIN_R, GPIO_IN);
  gpio_pull_up(BUT_PIN_R);

  // initialize Joystick
  //  set up gpio 4 for joystick button
  gpio_init(JOY_RIGHT); // right
  gpio_init(JOY_LEFT);  // left
  gpio_init(JOY_UP);    // up
  gpio_init(JOY_DOWN);  // down

  gpio_set_dir(JOY_RIGHT, GPIO_IN);
  gpio_set_dir(JOY_LEFT, GPIO_IN);
  gpio_set_dir(JOY_UP, GPIO_IN);
  gpio_set_dir(JOY_DOWN, GPIO_IN);

  // pullup ON, pulldown OFF
  gpio_pull_up(JOY_RIGHT);
  gpio_pull_up(JOY_LEFT);
  gpio_pull_up(JOY_UP);
  gpio_pull_up(JOY_DOWN);

  // Setup the ISR-timing GPIO
  gpio_init(ISR_GPIO);
  gpio_set_dir(ISR_GPIO, GPIO_OUT);
  gpio_put(ISR_GPIO, 0);

  // Enable the interrupt for the alarm (we're using Alarm 0)
  hw_set_bits(&timer_hw->inte, 1u << ALARM_NUM);
  // Associate an interrupt handler with the ALARM_IRQ
  irq_set_exclusive_handler(ALARM_IRQ, button_irq);
  // Enable the alarm interrupt
  irq_set_enabled(ALARM_IRQ, true);
  // Write the lower 32 bits of the target time to the alarm register, arming it.
  timer_hw->alarm[ALARM_NUM] = timer_hw->timerawl + DELAY;

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

  // Initialize SPI channel (channel, baud rate set to 20MHz)
  spi_init(SPI_PORT, 20000000);

  // Format SPI channel (channel, data bits per transfer, polarity, phase, order)
  spi_set_format(SPI_PORT, 16, SPI_CPOL_0, SPI_CPHA_0, SPI_LSB_FIRST);

  // Map SPI signals to GPIO ports, acts like framed SPI with this CS mapping
  gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
  gpio_set_function(PIN_CS, GPIO_FUNC_SPI);
  gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
  gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

  // Build sine table and DAC data table
  int i;
  for (i = 0; i < (sine_table_size); i++)
  {
      raw_sin[i] = (int)(2047 * sin((float)i * 6.283 / (float)sine_table_size) + 2047); // 12 bit
      DAC_data[i] = DAC_config_chan_A | (raw_sin[i] & 0x0fff);
  }

  int k;
  for (k = 0; k< splash_audio_len; k++){
      splash_data[k] = DAC_config_chan_A | (splash_audio[k] & 0x0FFF); //first convert to 12 bit then add config
  }

  int j;
  for (j = 0; j< boom_audio_len;j++){
      boom_data[j] = DAC_config_chan_A | (boom_audio[j] & 0x0FFF); //first convert to 12 bit then add config
  }

  //SPLASH
  // Select DMA channels
  data_chan_splash = dma_claim_unused_channel(true);
  ;
  ctrl_chan_splash = dma_claim_unused_channel(true);
  ;

  // Setup the control channel
  dma_channel_config c4 = dma_channel_get_default_config(ctrl_chan_splash); // default configs
  channel_config_set_transfer_data_size(&c4, DMA_SIZE_32);           // 32-bit txfers
  channel_config_set_read_increment(&c4, false);                     // no read incrementing
  channel_config_set_write_increment(&c4, false);                    // no write incrementing
  channel_config_set_chain_to(&c4, data_chan_splash);                       // chain to data channel

  dma_channel_configure(
      ctrl_chan_splash,                        // Channel to be configured
      &c4,                               // The configuration we just created
      &dma_hw->ch[data_chan_splash].read_addr, // Write address (data channel read address)
      &splash_pointer,                 // Read address (POINTER TO AN ADDRESS)
      1,                                // Number of transfers
      false                             // Don't start immediately
  );
 
  // Setup the data channel
  dma_channel_config c5 = dma_channel_get_default_config(data_chan_splash); // Default configs
  channel_config_set_transfer_data_size(&c5, DMA_SIZE_16);           // 16-bit txfers
  channel_config_set_read_increment(&c5, true);                      // yes read incrementing
  channel_config_set_write_increment(&c5, false);                    // no write incrementing
  // (X/Y)*sys_clk, where X is the first 16 bytes and Y is the second
  // sys_clk is 125 MHz unless changed in code. Configured to ~44 kHz
  // dma_timer_set_fraction(0, 0x0017, 0xffff);
//  dma_timer_set_fraction(0, 0x005, 0XDf00); // ~11025Hz
  dma_timer_set_fraction(0, 0x005, 0XDf00); // 16kHz

  // 0x3b means timer0 (see SDK manual)
  channel_config_set_dreq(&c5, 0x3b); // DREQ paced by timer 0
  // chain to the controller DMA channel
  // channel_config_set_chain_to(&c2, ctrl_chan); // Chain to control channel

  dma_channel_configure(
      data_chan_splash,                 // Channel to be configured
      &c5,                       // The configuration we just created
      &spi_get_hw(SPI_PORT)->dr, // write address (SPI data register)
      splash_data,                  // The initial read address
      splash_audio_len,           // Number of transfers
      false                      // Don't start immediately.
  );
 
  //BOOM
  // Select DMA channels
  data_chan_boom = dma_claim_unused_channel(true);
  ;
  ctrl_chan_boom = dma_claim_unused_channel(true);
  ;

  // Setup the control channel
  dma_channel_config c6 = dma_channel_get_default_config(ctrl_chan_boom); // default configs
  channel_config_set_transfer_data_size(&c6, DMA_SIZE_32);           // 32-bit txfers
  channel_config_set_read_increment(&c6, false);                     // no read incrementing
  channel_config_set_write_increment(&c6, false);                    // no write incrementing
  channel_config_set_chain_to(&c6, data_chan_boom);                       // chain to data channel

  dma_channel_configure(
      ctrl_chan_boom,                        // Channel to be configured
      &c6,                               // The configuration we just created
      &dma_hw->ch[data_chan_boom].read_addr, // Write address (data channel read address)
      &boom_pointer,                 // Read address (POINTER TO AN ADDRESS)
      1,                                // Number of transfers
      false                             // Don't start immediately
  );

  // Setup the data channel
  dma_channel_config c7 = dma_channel_get_default_config(data_chan_boom); // Default configs
  channel_config_set_transfer_data_size(&c7, DMA_SIZE_16);           // 16-bit txfers
  channel_config_set_read_increment(&c7, true);                      // yes read incrementing
  channel_config_set_write_increment(&c7, false);                    // no write incrementing
  // (X/Y)*sys_clk, where X is the first 16 bytes and Y is the second
  // sys_clk is 125 MHz unless changed in code. Configured to ~44 kHz
  // dma_timer_set_fraction(0, 0x0017, 0xffff);
  //  dma_timer_set_fraction(0, 0x005, 0XDf00); // ~11025Hz
  dma_timer_set_fraction(0,0x0008, 0xf500); // 16kHz

  // 0x3b means timer0 (see SDK manual)
  channel_config_set_dreq(&c7, 0x3b); // DREQ paced by timer 0
  // chain to the controller DMA channel
  // channel_config_set_chain_to(&c2, ctrl_chan); // Chain to control channel

  dma_channel_configure(
      data_chan_boom,                 // Channel to be configured
      &c7,                       // The configuration we just created
      &spi_get_hw(SPI_PORT)->dr, // write address (SPI data register)
      boom_data,                  // The initial read address
      boom_audio_len,           // Number of transfers
      false                      // Don't start immediately.
  );
  
  multicore_reset_core1();
  multicore_launch_core1(&core1_main);

  // sleep_ms(100);
  pt_add_thread(protothread_send);
  pt_add_thread(protothread_receive);

  // start scheduler
  pt_schedule_start;
}