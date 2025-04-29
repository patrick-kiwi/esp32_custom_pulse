#include <stdio.h>
#include <stdbool.h>
#include "driver/rmt_tx.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Pulse timing (microseconds)
#define PULSE_WIDTH_A 1
#define PULSE_DELAY_A 99
#define PULSE_WIDTH_B 2
#define PULSE_DELAY_B 98
#define PULSE_WIDTH_C 3
#define PULSE_DELAY_C 97
#define PULSE_WIDTH_D 4
#define PULSE_DELAY_D 96

// Tweaking the symbols_per_transaction +-1 changes the buffer size and the lag between channels 
// 50 ns lag (@ 1 MHz resolution) between channels is about as good as it gets
#define SYMBOLS_PER_TRANSACTION 9

static rmt_channel_handle_t tx_channels[4] = {NULL};
static gpio_num_t tx_gpio_number[] = {GPIO_NUM_19, GPIO_NUM_20, GPIO_NUM_21, GPIO_NUM_47 };

// Pulse Sequences
static rmt_symbol_word_t pulseSequenceA = {
    .duration0 = PULSE_WIDTH_A, .level0 = 1, .duration1 = PULSE_DELAY_A, .level1 = 0
};
static rmt_symbol_word_t pulseSequenceB = {
    .duration0 = PULSE_WIDTH_B, .level0 = 1, .duration1 = PULSE_DELAY_B, .level1 = 0
};
static rmt_symbol_word_t pulseSequenceC = {
    .duration0 = PULSE_WIDTH_C, .level0 = 1, .duration1 = PULSE_DELAY_C, .level1 = 0
};
static rmt_symbol_word_t pulseSequenceD = {
    .duration0 = PULSE_WIDTH_D, .level0 = 1, .duration1 = PULSE_DELAY_D, .level1 = 0
};

// Shared buffer for both channels
static rmt_symbol_word_t shared_buffer[SYMBOLS_PER_TRANSACTION * 4];

void IRAM_ATTR app_main(void) {
    // Increase task priority
    vTaskPrioritySet(NULL, configMAX_PRIORITIES - 1);

    // Configure and enable channels (change resolution to 10 MHz for increased precision)
    for (int i = 0; i < 4; i++) {
        rmt_tx_channel_config_t tx_chan_config = {
            .gpio_num = tx_gpio_number[i],
            .clk_src = RMT_CLK_SRC_DEFAULT,
            .resolution_hz = 1 * 1000 * 1000, // 1 MHz
            .mem_block_symbols = 48,
            .trans_queue_depth = 10,
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

    // Use a shared buffer for maximum speed
    for (int i = 0; i < SYMBOLS_PER_TRANSACTION; i++) {
        shared_buffer[i] = pulseSequenceA;                              // Channel 0
        shared_buffer[i + SYMBOLS_PER_TRANSACTION] = pulseSequenceB;    // Channel 1
        shared_buffer[i + 2*SYMBOLS_PER_TRANSACTION] = pulseSequenceC;  // Channel 2
        shared_buffer[i + 3*SYMBOLS_PER_TRANSACTION] = pulseSequenceD;  // Channel 3
    }

    rmt_transmit_config_t transmitConfig = {
        .loop_count = -1, // Continuous(-1) or single shot(0)
        .flags.eot_level = 0,  // End of transmission low
    };

    // Transmit different parts of the same buffer to all channels
    rmt_transmit(tx_channels[0], copy_encoder, shared_buffer, SYMBOLS_PER_TRANSACTION * sizeof(rmt_symbol_word_t), &transmitConfig);
    rmt_transmit(tx_channels[1], copy_encoder, shared_buffer + SYMBOLS_PER_TRANSACTION, SYMBOLS_PER_TRANSACTION * sizeof(rmt_symbol_word_t), &transmitConfig);
    rmt_transmit(tx_channels[2], copy_encoder, shared_buffer + 2*SYMBOLS_PER_TRANSACTION, SYMBOLS_PER_TRANSACTION * sizeof(rmt_symbol_word_t), &transmitConfig);
    rmt_transmit(tx_channels[3], copy_encoder, shared_buffer + 3*SYMBOLS_PER_TRANSACTION, SYMBOLS_PER_TRANSACTION * sizeof(rmt_symbol_word_t), &transmitConfig);
}