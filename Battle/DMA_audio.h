#ifndef DMA_AUDIO.H
#define DMA_AUDIO .H

/**
 *  V. Hunter Adams (vha3)
    Code based on examples from Raspberry Pi Co

    Sets up two DMA channels. One sends samples at audio rate to the DAC,
    (data_chan), and the other writes to the data_chan DMA control registers (ctrl_chan).

    The control channel writes to the data channel, sending one period of
    a sine wave thru the DAC. The control channel is chained to the data
    channel, so it is re-triggered after the data channel is finished. The data
    channel is chained to the control channel, so it starts as soon as the
    control channel is finished. The control channel resets the read address
    of the data channel.
 */

#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/spi.h"

#include "splash.h"
//  #include "boom.h"
#include "boom.h"

// Number of samples per period in sine table
#define sine_table_size 256

// Sine table
int raw_sin[sine_table_size];

// Table of values to be sent to DAC
unsigned short DAC_data[sine_table_size];
unsigned short splash_data[splash_audio_len];
unsigned short boom_data[boom_audio_len];

// Pointer to the address of the DAC data table
// unsigned short *address_pointer = &DAC_data[0];
unsigned short *splash_pointer = &splash_data[0];
unsigned short *boom_pointer = &boom_data[0];

// A-channel, 1x, active
#define DAC_config_chan_A 0b0011000000000000

// SPI configurations
#define PIN_MISO 4
#define PIN_CS 5
#define PIN_SCK 6
#define PIN_MOSI 7
#define SPI_PORT spi0

// Number of DMA transfers per event
const uint32_t transfer_count = sine_table_size;

int data_chan_splash;
int ctrl_chan_splash;
int data_chan_boom;
int ctrl_chan_boom;

#endif /*DMA_ADUIO.H*/