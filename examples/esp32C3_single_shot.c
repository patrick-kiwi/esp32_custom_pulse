#include <stdio.h>
#include <stdbool.h>
#include "driver/rmt_tx.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Pulse timing (microseconds)
#define PULSE_WIDTH_A 1
#define PULSE_DELAY_A 20
#define PULSE_WIDTH_B 2
#define PULSE_DELAY_B 20

static rmt_channel_handle_t tx_channels[2] = {NULL};
static gpio_num_t tx_gpio_number[] = {GPIO_NUM_7, GPIO_NUM_21 };

// Pulse Sequences
static rmt_symbol_word_t pulseSequenceA = {
    .duration0 = PULSE_WIDTH_A, .level0 = 1, .duration1 = PULSE_DELAY_A, .level1 = 0
};
static rmt_symbol_word_t pulseSequenceB = {
    .duration0 = PULSE_WIDTH_B, .level0 = 1, .duration1 = PULSE_DELAY_B, .level1 = 0
};


void app_main(void) {

    // Configure and enable channels (change resolution to 10 MHz for increased precision)
    for (int i = 0; i < 2; i++) {
        rmt_tx_channel_config_t tx_chan_config = {
            .gpio_num = tx_gpio_number[i],
            .clk_src = RMT_CLK_SRC_DEFAULT,
            .resolution_hz = 1 * 1000 * 1000, // 1 MHz
            .mem_block_symbols = 48,
            .trans_queue_depth = 2,
        };
        ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &tx_channels[i]));
        ESP_ERROR_CHECK(rmt_enable(tx_channels[i]));
    }

    // Create sync manager
    rmt_sync_manager_handle_t synchro = NULL;
    rmt_sync_manager_config_t synchro_config = {
        .tx_channel_array = tx_channels,
        .array_size = sizeof(tx_channels) / sizeof(tx_channels[0]),
    };
    ESP_ERROR_CHECK(rmt_new_sync_manager(&synchro_config, &synchro));
    
    // Create copy encoder
    rmt_encoder_handle_t copy_encoder = NULL;
    rmt_copy_encoder_config_t copy_config = {};
    ESP_ERROR_CHECK(rmt_new_copy_encoder(&copy_config, &copy_encoder));


    rmt_transmit_config_t transmitConfig = {
        .loop_count = 0, // Continuous(-1) or single shot(0)
        .flags.eot_level = 0,  // End of transmission low
    };

    // Transmit 
    while(1) {
    rmt_transmit(tx_channels[0], copy_encoder, &pulseSequenceA,  sizeof(pulseSequenceA), &transmitConfig);
    rmt_transmit(tx_channels[1], copy_encoder, &pulseSequenceB, sizeof(pulseSequenceB), &transmitConfig);
    rmt_sync_reset(synchro);
    }
}
