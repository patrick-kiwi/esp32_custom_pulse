#include "driver/rmt_tx.h"
#include "freertos/FreeRTOS.h"

// 10 microsecond pulse @ 2KHz
rmt_symbol_word_t pulsePatternA[] = {
    { .duration0 = 10, .level0 = 1, .duration1 = 480, .level1 = 0 },   
};

/* 
Synchronise a 3 microsecond pulse to the trailing edge of the above pulse
To prevent problems, make the tick number the same, or slightly less the same as for patternA
by adding a dummy symbol to padd the tick number
*/
rmt_symbol_word_t pulsePatternB[] = {
    { .duration0 = 10, .level0 = 0, .duration1 = 3, .level1 = 1 },
    { .duration0 = 400, .level0 = 0, .duration1 = 0, .level1 = 0 },
    
};

static rmt_channel_handle_t tx_channels[2] = {NULL};
static gpio_num_t tx_gpio_number[2] = {GPIO_NUM_7, GPIO_NUM_21};

void app_main(void) {
    // Initialize RMT channels
    for (int i = 0; i < 2; i++) {
        rmt_tx_channel_config_t tx_chan_config = {
            .gpio_num = tx_gpio_number[i],
            .clk_src = RMT_CLK_SRC_DEFAULT,
            .resolution_hz = 1 * 1000 * 1000,
            .mem_block_symbols = 48,
            .trans_queue_depth = 8,
        };
        ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &tx_channels[i]));
    }

    for (int i = 0; i < 2; i++) {
        ESP_ERROR_CHECK(rmt_enable(tx_channels[i]));
    }

    // Define synch manager
    rmt_sync_manager_handle_t synchro = NULL;
    rmt_sync_manager_config_t synchro_config = {
        .tx_channel_array = tx_channels,
        .array_size = sizeof(tx_channels) / sizeof(tx_channels[0]),
    };
    ESP_ERROR_CHECK(rmt_new_sync_manager(&synchro_config, &synchro));

    // Define transmit configuration
    rmt_transmit_config_t transmitConfig = {
        .loop_count = 0,
    };
    
    // Define a single copy encoder
    rmt_encoder_handle_t copyEncoder = NULL;
    rmt_copy_encoder_config_t copyEncoderConfig = {};
    assert(rmt_new_copy_encoder(&copyEncoderConfig, &copyEncoder) == ESP_OK && "Failed to Create Copy Encoder");

    // Transmit and reset loop
    while(1) {
    rmt_transmit(tx_channels[0], copyEncoder, pulsePatternA, sizeof(pulsePatternA), &transmitConfig);
    rmt_transmit(tx_channels[1], copyEncoder, pulsePatternB, sizeof(pulsePatternB), &transmitConfig);
    rmt_sync_reset(synchro);
}
}
