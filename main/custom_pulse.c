#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include "driver/rmt_tx.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Pulse characteristics in microseconds
#define PULSE_WIDTH 10
#define PULSE_DELAY 90

static rmt_channel_handle_t tx_channels = { NULL };
static gpio_num_t tx_gpio_number = GPIO_NUM_19;
static rmt_encoder_handle_t copyEncoder = NULL;

void app_main(void) {
    // Initialize RMT channels
    printf("setting up channel\n");
    rmt_tx_channel_config_t tx_chan_config = {
      .gpio_num = tx_gpio_number,
      .clk_src = RMT_CLK_SRC_DEFAULT,
      .resolution_hz = 1 * 1000 * 1000,  // 1MHz clock
      .mem_block_symbols = 48,      
      .trans_queue_depth = 1,
    };
    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &tx_channels));
  
    rmt_transmit_config_t transmitConfig = { .loop_count = -1 };
    rmt_encoder_handle_t copyEncoder =NULL;
    rmt_copy_encoder_config_t copyEncoderConfig = {};
    assert(rmt_new_copy_encoder(&copyEncoderConfig, &copyEncoder) == ESP_OK && "Failed to Create Copy Encoder");

    // Enable all channels
    ESP_ERROR_CHECK(rmt_enable(tx_channels));

    // Define initial pulse patterns
    rmt_symbol_word_t pulsePattern1[] = {
        { .duration0 = PULSE_WIDTH, .level0 = 1, .duration1 = PULSE_DELAY, .level1 = 0 },
        
    };

    // Reset synchronization and start transmissions
    ESP_ERROR_CHECK(rmt_transmit(tx_channels, copyEncoder, &pulsePattern1, sizeof(pulsePattern1), &transmitConfig));
}
