# Esp32 Simple Pulse Sequence Synchronisation
You can’t synchronise pulses on a standard ESP32 because the board doesn’t have a sync manager.  The following table summarizes the RMT hardware limitations of different esp32 boards.
```
Board     | No. RMT TX Ch# | Sync Manager Supported?
ESP32     | 8              | NO
ESP32-S3  | 4              | YES
ESP32-C3  | 2              | YES
```
### Limitations – Infinite loop approach
Sending a unique buffer to different channels with .loop_count=-1 results in imperfect synchronisation, specifically a phase shift of at least 50 ns between consecutive channels.  Experimentally, this can be minimized by trying a combination of the following; increasing the .resolution_hz from 1 to 10 MHz, adjusting the SYMBOLS_PER_TRANSACTION from 1 to 10, changing the .trans_queue_depth from 1 to 10, or by commenting out the vTaskPrioritySet line.  Be aware that the rmt_word_symbol has only 15 bits to encode “ticks”, meaning that at 10 MHz the total on-and-off time of a single pulse can’t greater than 1/10th of a microsecond * 2^15 ticks which equals 3.2 ms (ie. minimum frequency=305 Hz @ 10 MHz tick resolution)
With .loop_count=-1, the pulses being sent to different channels must also have an identical total number of ticks, otherwise they’ll shift out of phase every cycle.  The infinite loop approach gives you precise control over the frequency, and results in zero processor overhead, but that comes at the expense phase shift imprecision, and never being able to call “rmt_tx_event_callbacks_t::on_trans_done” or rmt_sync_reset because the transmission is never done.  

### Limitations - Single shot approach
Setting .loop_count=0 results in single-shot pulses being sent to each respective channel. The transmit calls need to be followed by a rmt_sync_reset(), and that all needs to be to be encapsulated in infinite while(1){} loop.  The synchronisation is perfect, and the pulse sequences can have arbitrary different lengths.  You can also use callbacks at the end of each sequence to do something.  But, that all comes at the expense of processor cycles, and a loss in ability to control the precise frequency. 
