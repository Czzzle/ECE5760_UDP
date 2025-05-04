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
// Protothreads
#include "pt_cornell_rp2040_v1_3.h"

// BattleshipGame setting
#include "BattleshipGame.h"

// Destination port and IP address
#define UDP_PORT 1234
#define BEACON_TARGET "172.20.10.2" // after connect to akansha internet //"172.20.10.2"

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

static void udpecho_raw_recv(void *arg, struct udp_pcb *upcb, struct pbuf *p,
                             const ip_addr_t *addr, u16_t port)
{
  LWIP_UNUSED_ARG(arg);

  // Check that there's something in the pbuf
  if (p != NULL)
  {
    // Copy the contents of the payload
    effective_len = p->len;
    printf("length of msg %d\n", effective_len);
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

/*
static void raw_send(char send_data[], GAME_STATUS status, GRID_STATE state, Coordinate8 coord, int sendOption)
{
  //decode information
  switch(sendOption){
    case 1:
      //send Game Status
      switch (status) {
        case GAME_STATUS::INITIAL:
          send_data = "GAMEINITIAL";
          break;
        case GAME_STATUS::LEVEL:
          send_data = "GAMELEVEL";
          break;
        case GAME_STATUS::PLACE:
          send_data = "GAMEPLACE";
          break;

        case GAME_STATUS::ONGOING:
          send_data = "GAMEONGOING";
          break;
        case GAME_STATUS::WIN:
          send_data = "GAMEWIN";
          break;
        case GAME_STATUS::LOSE:
          send_data = "GAMELOSE";
          break;
      }
      break;
    case 2:
      //send Grid state
      switch (state) {
        case GRID_STATE::WATER:
          send_data = "GRIDWATER";
          break;
        case GRID_STATE::SHIP:
          send_data = "GRIDSHIP";
          break;
        case GRID_STATE::HIT:
          send_data = "GRIDHIT";
          break;

        case GRID_STATE::MISS:
          send_data = "GRIDMISS";
          break;

      }
      break;

    case 3:
      //send coord (need to encode form coordinates8 to char*)
      char* out;
      encodeCoord(coord, out);
      send_data = out;
  }

  PT_SEM_SAFE_SIGNAL(pt, &ready_to_send); //send sephmore

}
*/
static void raw_send_test()
{
  sleep_ms(4000);
  strcpy(send_data, "HELLO");
  printf("%s", send_data);
  PT_SEM_SAFE_SIGNAL(pt, &ready_to_send);

  sleep_ms(1000);
  strcpy(send_data, "YEAH");
  PT_SEM_SAFE_SIGNAL(pt, &ready_to_send);

  sleep_ms(1000);
  strcpy(send_data, "12344");
  PT_SEM_SAFE_SIGNAL(pt, &ready_to_send);
}
// ===================================
// Define the recv callback
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

// This function is a "sudo" function so far since I cannot use enum class in C file
// void decodeComingMsg(char received_data[], int effective_len)
// {

//   if (effective_len <= 3){ // we know received data is  coordinate, we need to check hit or miss
//     //decode "A1" style coordinate
//     //check the hit or miss
//     //send back hit or miss msg
//   }
//   else if(strncmp(received_data, "GAME", 4) == 0){
//     // record other player's game status
//     char *status = received_data + 4;
//     if (strcmp(status, "INITIAL") == 0){
//       // status initial
//     }
//     else if(strcmp(status, "LEVEL") == 0){
//       //LEVEL
//     }
//     else if(strcmp(status, "PLACE") == 0){
//       //place
//     }
//     else if(strcmp(status, "ONGOING") == 0){
//       //ONGOING
//     }
//     else if(strcmp(status, "WIN") == 0){
//       //WIN
//     }
//     else {
//       //LOSE
//     }
//   }else{
//     char *state = received_data + 4;
//     if (strcmp(state, "WATER") == 0){
//       // water
//     }
//     else if(strcmp(state, "SHIP") == 0){
//       //ship
//     }
//     else if(strcmp(state, "HIT") == 0){
//       //hit
//     }
//     else{
//       //miss
//     }
//   }

// }

static PT_THREAD(protothread_receive(struct pt *pt))
{
  // Begin thread
  PT_BEGIN(pt);

  while (1)
  {
    // Wait on a semaphore
    PT_SEM_SAFE_WAIT(pt, &new_message);

    // Print received message
    printf("%s\n", received_data);

    // Check the content of the received_data
    if (strcmp(received_data, "Recieved") == 0)
    {
      printf("test for equal comparison");
    }
  }

  // End thread
  PT_END(pt);
}

// This thread is a test for main thread when calling send
static PT_THREAD(protothread_mainHold(struct pt *pt))
{
  // Begin thread
  PT_BEGIN(pt);

  raw_send_test();

  // End thread
  PT_END(pt);
}

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

    PT_SEM_SAFE_WAIT(pt, &ready_to_send);

    // // Prompt the user
    // sprintf(pt_serial_out_buffer, "> ");
    // serial_write;

    // // Perform a non-blocking serial read for a string
    // serial_read;

    // Allocate a PBUF
    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, BEACON_MSG_LEN_MAX + 1, PBUF_RAM);

    // Pointer to the payload of the pbuf
    char *req = (char *)p->payload;

    // Clear the pbuf payload
    memset(req, 0, BEACON_MSG_LEN_MAX + 1);

    // Fill the pbuf payload
    snprintf(req, BEACON_MSG_LEN_MAX, "%s: %s \n",
             ip4addr_ntoa(netif_ip_addr4(netif_default)), send_data);

    // Send the UDP packet
    err_t er = udp_sendto(pcb, p, &addr, UDP_PORT);

    printf("Send UDP packet");

    // Free the PBUF
    pbuf_free(p);

    // Check for errors
    if (er != ERR_OK)
    {
      printf("Failed to send UDP packet! error=%d", er);
    }
    PT_SEM_SAFE_SIGNAL(pt, &ready_to_send); 
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

  // Add threads, start scheduler
  pt_add_thread(protothread_send);
  pt_add_thread(protothread_receive);
  pt_add_thread(protothread_mainHold);

  pt_schedule_start;

  return 0;
}