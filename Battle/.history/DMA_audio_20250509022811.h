#ifndef DMA_AUDIO.H
#define DMA_AUDIO.H

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

// Initidalize stdio
    stdio_init_all();

    // Initialize SPI channel (channel, baud rate set to 20MHz)
    spi_init(SPI_PORT, 20000000);

    // Format SPI channel (channel, data bits per transfer, polarity, phase, order)
    spi_set_format(SPI_PORT, 16, 0, 0, 0);

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
    int data_chan_splash = dma_claim_unused_channel(true);
    ;
    int ctrl_chan_splash = dma_claim_unused_channel(true);
    ;

    // Setup the control channel
    dma_channel_config c1 = dma_channel_get_default_config(ctrl_chan_splash); // default configs
    channel_config_set_transfer_data_size(&c1, DMA_SIZE_32);           // 32-bit txfers
    channel_config_set_read_increment(&c1, false);                     // no read incrementing
    channel_config_set_write_increment(&c1, false);                    // no write incrementing
    channel_config_set_chain_to(&c1, data_chan_splash);                       // chain to data channel

    dma_channel_configure(
        ctrl_chan_splash,                        // Channel to be configured
        &c1,                               // The configuration we just created
        &dma_hw->ch[data_chan_splash].read_addr, // Write address (data channel read address)
        &splash_pointer,                 // Read address (POINTER TO AN ADDRESS)
        1,                                // Number of transfers
        false                             // Don't start immediately
    );

    // Setup the data channel
    dma_channel_config c2 = dma_channel_get_default_config(data_chan_splash); // Default configs
    channel_config_set_transfer_data_size(&c2, DMA_SIZE_16);           // 16-bit txfers
    channel_config_set_read_increment(&c2, true);                      // yes read incrementing
    channel_config_set_write_increment(&c2, false);                    // no write incrementing
    // (X/Y)*sys_clk, where X is the first 16 bytes and Y is the second
    // sys_clk is 125 MHz unless changed in code. Configured to ~44 kHz
    // dma_timer_set_fraction(0, 0x0017, 0xffff);
//  dma_timer_set_fraction(0, 0x005, 0XDf00); // ~11025Hz
    dma_timer_set_fraction(0, 0x005, 0XDf00); // 16kHz

    // 0x3b means timer0 (see SDK manual)
    channel_config_set_dreq(&c2, 0x3b); // DREQ paced by timer 0
    // chain to the controller DMA channel
    // channel_config_set_chain_to(&c2, ctrl_chan); // Chain to control channel

    dma_channel_configure(
        data_chan_splash,                 // Channel to be configured
        &c2,                       // The configuration we just created
        &spi_get_hw(SPI_PORT)->dr, // write address (SPI data register)
        splash_data,                  // The initial read address
        splash_audio_len,           // Number of transfers
        false                      // Don't start immediately.
    );

    // start the control channel
    dma_start_channel_mask(1u << ctrl_chan_splash);


//BOOM

// Select DMA channels
    int data_chan_boom = dma_claim_unused_channel(true);
    ;
    int ctrl_chan_boom = dma_claim_unused_channel(true);
    ;

    // Setup the control channel
    dma_channel_config c3 = dma_channel_get_default_config(ctrl_chan_boom); // default configs
    channel_config_set_transfer_data_size(&c3, DMA_SIZE_32);           // 32-bit txfers
    channel_config_set_read_increment(&c3, false);                     // no read incrementing
    channel_config_set_write_increment(&c3, false);                    // no write incrementing
    channel_config_set_chain_to(&c3, data_chan_boom);                       // chain to data channel

    dma_channel_configure(
        ctrl_chan_boom,                        // Channel to be configured
        &c3,                               // The configuration we just created
        &dma_hw->ch[data_chan_boom].read_addr, // Write address (data channel read address)
        &boom_pointer,                 // Read address (POINTER TO AN ADDRESS)
        1,                                // Number of transfers
        false                             // Don't start immediately
    );

    // Setup the data channel
    dma_channel_config c4 = dma_channel_get_default_config(data_chan_boom); // Default configs
    channel_config_set_transfer_data_size(&c4, DMA_SIZE_16);           // 16-bit txfers
    channel_config_set_read_increment(&c4, true);                      // yes read incrementing
    channel_config_set_write_increment(&c4, false);                    // no write incrementing
    // (X/Y)*sys_clk, where X is the first 16 bytes and Y is the second
    // sys_clk is 125 MHz unless changed in code. Configured to ~44 kHz
    // dma_timer_set_fraction(0, 0x0017, 0xffff);
//  dma_timer_set_fraction(0, 0x005, 0XDf00); // ~11025Hz
    dma_timer_set_fraction(0,0x0008, 0xf500); // 16kHz

    // 0x3b means timer0 (see SDK manual)
    channel_config_set_dreq(&c4, 0x3b); // DREQ paced by timer 0
    // chain to the controller DMA channel
    // channel_config_set_chain_to(&c2, ctrl_chan); // Chain to control channel

    dma_channel_configure(
        data_chan_boom,                 // Channel to be configured
        &c4,                       // The configuration we just created
        &spi_get_hw(SPI_PORT)->dr, // write address (SPI data register)
        boom_data,                  // The initial read address
        boom_audio_len,           // Number of transfers
        false                      // Don't start immediately.
    );

    // start the control channel
    dma_start_channel_mask(1u << ctrl_chan_boom);




#endif /*DMA_ADUIO.H*/