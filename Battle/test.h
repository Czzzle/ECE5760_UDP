/**
 * Copyright (c) 2022 Andrew McDonnell
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// Standard libraries
#include <string.h>
#include <stdio.h>
#include "pico/stdlib.h"
// LWIP libraries
#include "lwip/pbuf.h"
#include "lwip/udp.h"
// Pico SDK hardware support libraries
#include "hardware/sync.h"
// Our header for making WiFi connection
#include "connect.h"

// BattleshipGame setting
#include "BattleshipGame.h"

// Destination port and IP address
#define UDP_PORT 1234
#define BEACON_TARGET "172.20.10.3" // after connect to akansha internet //"172.20.10.2"
// #define BEACON_TARGET "172.20.10.2"

// Maximum length of our message
#define BEACON_MSG_LEN_MAX 127

// Protocol control block for UDP receive connection
static struct udp_pcb *udp_rx_pcb;

// Buffer in which to copy received messages and effective msg length
char received_data[BEACON_MSG_LEN_MAX];
int effective_len; // for received data

// Buffer in which for send data
char send_data[BEACON_MSG_LEN_MAX];

// Semaphore for signaling a new received message
struct pt_sem new_message;

// Semaphore for signaling a msg need to be sent out
struct pt_sem ready_to_send;

// Other player status
GAME_STATUS oponent_player = GAME_STATUS::ONGOING; // remove this later
GRID_STATE opponent_gridstate;
Coordinate8 our_shippos;

bool start_game = false;
bool your_turn = true; // true for player 1 and false for player 2
bool received_response = false;
bool received_attack = false; // false for both the players

void encoderCoord(Coordinate8 c, char *out, size_t out_size)
{
    if (out == NULL || out_size < 4)
    {
        // Defensive: can't encode
        return;
    }
    out[0] = 'A' + c.x;
    snprintf(out + 1, out_size - 1, "%d", c.y + 1);
}

// Coordinate8 decodeCoordUDP(const char* msg, int effective_len) {
//   Coordinate8 coord = { .x = 30, .y = 30 }; // Default invalid

//   if (msg == NULL || effective_len < 3) return coord;

//   // Assume coordinate is at the end of the message
//   const char* coord_str = msg + effective_len - 3;  // Try last 3 chars
//   char buffer[4] = {0};

//   if (isdigit(coord_str[1]) && isdigit(coord_str[2])) {
//       // Case: A10
//       strncpy(buffer, coord_str, 3);  // 3 chars + null
//   } else if (isdigit(coord_str[1])) {
//       // Case: A5
//       coord_str = msg + effective_len - 2;
//       strncpy(buffer, coord_str, 2);  // 2 chars + null
//   } else {
//       return coord;  // Invalid format
//   }

//   return decodeCoord(buffer);  // Use your existing function
// }

static void raw_send(GAME_STATUS status, GRID_STATE state, Coordinate8 coord, int sendOption)
{
    // decode information
    switch (sendOption)
    {
    case 1:
        // send Game Status
        switch (status)
        {
        case GAME_STATUS::INITIAL:
            strcpy(send_data, "GAMEINITIAL");
            break;
        case GAME_STATUS::LEVEL:
            strcpy(send_data, "GAMELEVEL");
            break;
        case GAME_STATUS::PLACE:
            strcpy(send_data, "GAMEPLACE");
            break;
        case GAME_STATUS::ONGOING:
            strcpy(send_data, "GAMEONGOING");
            break;
        case GAME_STATUS::WIN:
            strcpy(send_data, "GAMEWIN");
            break;
        case GAME_STATUS::LOSE:
            strcpy(send_data, "GAMELOSE");
            break;
        }
        break;
    case 2:
        // send Grid state
        switch (state)
        {
        case GRID_STATE::WATER:
            strcpy(send_data, "GRIDWATER");
            break;
        // case GRID_STATE::SHIP:
        // strcpy(send_data, "GRIDSHIP");
        // break;
        case GRID_STATE::HIT:
            strcpy(send_data, "GRIDHIT");
            break;
        case GRID_STATE::MISS:
            strcpy(send_data, "GRIDMISS");
            break;
        case GRID_STATE::REPEAT:
            strcpy(send_data, "GRIDREPEAT");
            break;
        }
        break;

    case 3:
        // send coord (need to encode form coordinates8 to char*)
        char out[4];
        encoderCoord(coord, out, sizeof(out));
        printf("\nsending data out:%s", out);
        strcpy(send_data, out);
    }
    PT_SEM_SAFE_SIGNAL(pt, &ready_to_send); // send sephmore
}

// ===================================
// DECODE MESSAGE
// ===================================
// Decodes a coordinate from "A5" → (0,4)
Coordinate8 decodeCoord(const char *code)
{
    Coordinate8 c;
    c.x = toupper(code[0]) - 'A';
    c.y = atoi(code + 1) - 1;
    return c;
}

void decodeComingMsg(char received_data[], int effective_len)
{
    printf("\nReceiveddata: %s", received_data);
    // printf("\neffective: %d",effective_len);
    int length_string = strlen(received_data);
    // printf("\nReceiveddata Length: %d",length_string);
    // for(int i=0;i<length_string;i++)
    // {
    //   printf("Received_data[%d]: %c",i,received_data[i]);
    // }
    if (length_string <= 18 && length_string > 16)
    {
        if (length_string == 17)
        {
            char coord_str[3];                // 2 characters + null
            coord_str[0] = received_data[13]; // 14th character (index 13)
            coord_str[1] = received_data[14]; // 15th character (index 14)
            coord_str[2] = '\0';              // null terminator
            printf("\nCoord_Str1: %s", coord_str);
            our_shippos = decodeCoord(coord_str);
        }
        else if (length_string == 18)
        {
            char coord_str[4]; // 2 characters + null
            coord_str[0] = received_data[13];
            coord_str[1] = received_data[14]; // 14th character (index 13)
            coord_str[2] = received_data[15]; // 15th character (index 14)
            coord_str[3] = '\0';              // null terminator
            printf("\nCoord_Str2: %s", coord_str);
            our_shippos = decodeCoord(coord_str);
        }
        printf("Decode msg: coordinate: x = %d, y%d\n", our_shippos.x, our_shippos.y);
    }
    else if (strstr(received_data, "GAME") != NULL)
    {
        if (strstr(received_data, "INITIAL") != NULL)
        {
            oponent_player = GAME_STATUS::INITIAL;
        }
        else if (strstr(received_data, "LEVEL") != NULL)
        {
            // LEVEL
            oponent_player = GAME_STATUS::LEVEL;
        }
        else if (strstr(received_data, "PLACE") != NULL)
        {
            // place
            oponent_player = GAME_STATUS::PLACE;
        }
        else if (strstr(received_data, "ONGOING") != NULL)
        {
            // ONGOING
            oponent_player = GAME_STATUS::ONGOING;
        }
        else if (strstr(received_data, "WIN") != NULL)
        {
            // WIN
            oponent_player = GAME_STATUS::WIN;
        }
        else
        {
            // LOSE
            oponent_player = GAME_STATUS::LOSE;
        }
        printf("\nOpponent_player: %d", oponent_player);
    }
    else if (strstr(received_data, "GRID") != NULL)
    {
        if (strstr(received_data, "REPEAT") != NULL)
        {
            opponent_gridstate = GRID_STATE::REPEAT;
            your_turn = true;
        }
        else if (strstr(received_data, "MISS") != NULL)
        {
            // ship
            opponent_gridstate = GRID_STATE::MISS;
        }
        else if (strstr(received_data, "HIT") != NULL)
        {
            // hit
            opponent_gridstate = GRID_STATE::HIT;
        }
        else
        {
            printf("\nInvalid grid value");
        }
        printf("\nopponent_gridstate: %d", opponent_gridstate);
    }
    else
    {
        printf("\nInvalid attack sequence");
        // Coordinate8 posnx;
        // posnx.x = 30;
        // posnx.y = 30;
        // raw_send(GAME_STATUS::ONGOING, GRID_STATE::REPEAT, posnx, 2);
        // raw_send(GAME_STATUS::ONGOING, GRID_STATE::REPEAT, posnx, 1);
    }
}

// ===================================
// UDP ECHO RAW INIT
// ===================================
// void udpecho_raw_init(void);
static void udpecho_raw_recv(void *arg, struct udp_pcb *upcb, struct pbuf *p,
                             const ip_addr_t *addr, u16_t port)
{
    LWIP_UNUSED_ARG(arg);

    // Check that there's something in the pbuf
    if (p != NULL)
    {
        // Copy the contents of the payload
        effective_len = p->len;
        // printf("length of msg %d\n", effective_len);
        // memcpy(received_data, p->payload, BEACON_MSG_LEN_MAX); // this line will copy all payload into received_data
        memcpy(received_data, p->payload, effective_len); // this line will copy effective payload into received_data
        // Semaphore-signal a thread
        PT_SEM_SAFE_SIGNAL(pt, &new_message);
        // Reset the payload buffer
        memset(p->payload, 0, BEACON_MSG_LEN_MAX + 1);
        // Free the PBUF
        pbuf_free(p);
    }
    else
        printf("NULL pt in callback");
}

static void raw_send_test()
{
    sleep_ms(4000);
    strcpy(send_data, "A10");
    printf("%s", send_data);
    PT_SEM_SAFE_SIGNAL(pt, &ready_to_send);

    sleep_ms(1000);
    strcpy(send_data, "GAMEONGOING");
    printf("%s", send_data);
    PT_SEM_SAFE_SIGNAL(pt, &ready_to_send);

    sleep_ms(1000);
    strcpy(send_data, "GRIDMISS");
    printf("%s", send_data);
    PT_SEM_SAFE_SIGNAL(pt, &ready_to_send);
}
// ===================================
// Define the recv callback
// ===================================
void udpecho_raw_init(void)
{
    // Initialize the RX protocol control block
    udp_rx_pcb = udp_new_ip_type(IPADDR_TYPE_ANY);
    // Make certain that the pcb has initialized, else print a message
    if (udp_rx_pcb != NULL)
    {
        // Err_t object for error codes
        err_t err;
        // Bind this PCB to our assigned IP, and our chosen port. Received messages
        // to this port will be directed to our bound protocol control block.
        err = udp_bind(udp_rx_pcb, netif_ip_addr4(netif_default), UDP_PORT);
        // Check that the bind was successful, else print a message
        if (err == ERR_OK)
        {
            // Setup the receive callback function
            udp_recv(udp_rx_pcb, udpecho_raw_recv, NULL);
        }
        else
        {
            printf("bind error");
        }
    }
    else
    {
        printf("udp_rx_pcb error");
    }
}

// ==================================================
// === PROTOTHREAD SEND
// ==================================================
static PT_THREAD(protothread_send(struct pt *pt))
{
    // Begin thread
    PT_BEGIN(pt);

    // Make a static local UDP protocol control block
    static struct udp_pcb *pcb;
    // Initialize that protocol control block
    pcb = udp_new();

    // Create a static local IP_ADDR_T object
    static ip_addr_t addr;
    // Set the value of this IP address object to our destination IP address
    ipaddr_aton(BEACON_TARGET, &addr);

    while (1)
    {

        // PT_SEM_SAFE_WAIT(pt, &ready_to_send);

        // // Prompt the user
        sprintf(pt_serial_out_buffer, "> ");
        serial_write;

        // Perform a non-blocking serial read for a string
        serial_read;

        // Allocate a PBUF
        struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, BEACON_MSG_LEN_MAX + 1, PBUF_RAM);

        // Pointer to the payload of the pbuf
        char *req = (char *)p->payload;

        // Clear the pbuf payload
        memset(req, 0, BEACON_MSG_LEN_MAX + 1);

        // Fill the pbuf payload
        snprintf(req, BEACON_MSG_LEN_MAX, "%s: %s \n",
                 ip4addr_ntoa(netif_ip_addr4(netif_default)), pt_serial_in_buffer);

        // Send the UDP packet
        err_t er = udp_sendto(pcb, p, &addr, UDP_PORT);

        printf("\nSend UDP packet");

        // Free the PBUF
        pbuf_free(p);

        // Check for errors
        if (er != ERR_OK)
        {
            printf("\nFailed to send UDP packet! error=%d", er);
        }
        // PT_SEM_SAFE_SIGNAL(pt, &ready_to_send);
    }
    // End thread
    PT_END(pt);
}

// ==================================================
// === PROTOTHREAD RECEIVE
// ==================================================

static PT_THREAD(protothread_receive(struct pt *pt))
{
    // Begin thread
    PT_BEGIN(pt);

    while (1)
    {
        // Wait on a semaphore
        PT_SEM_SAFE_WAIT(pt, &new_message);
        decodeComingMsg(received_data, effective_len);
    }

    // End thread
    PT_END(pt);
}

int main()
{

    // Initialize stdio
    stdio_init_all();
    sleep_ms(2000);
    printf("Start working\n");
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

    // raw_send_test();
    Coordinate8 coord;
    coord.x = 2;
    coord.y = 2;
    raw_send(GAME_STATUS::ONGOING, GRID_STATE::SHIP, coord, 1);
    // raw_send(send_data, GAME_STATUS status, GRID_STATE state, Coordinate8 coord, int sendOption)

    // Add threads, start scheduler
    pt_add_thread(protothread_send);
    pt_add_thread(protothread_receive);
    // pt_add_thread(protothread_mainHold);

    pt_schedule_start;

    return 0;
}
