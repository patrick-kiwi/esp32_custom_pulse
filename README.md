# Esp32 Simple Pulse Sequence Synchronisation
The following table summarizes the RMT hardware limitations of different esp32 boards.
```
Board     | Number of RMT Channels | Sync Manager Supported?
ESP32     |     8                  |     NO
ESP32-S3  |     4                  |     YES
ESP32-C3  |     2                  |     YES
```
### Limitations – Infinite loop approach
Sending a unique buffer to different channels with .loop_count=-1 results in imperfect synchronisation, specifically a phase shift of at least 50 to 300 nanoseconds between consecutive channels.  The phase shift can be minimized down to ~50 ns by varying .resolution_hz from 1 to 10 MHz, adjusting SYMBOLS_PER_TRANSACTION from 1 to 10, changing the .trans_queue_depth from 1 to 10, or by commenting out the vTaskPrioritySet line.  The rmt_word_symbol has only 15 bits to encode a maximum of 32768 “ticks” per rmt_symbol_word_t.

With .loop_count=-1 the total number of ticks being sent to different channels must be identical, otherwise the channels will shift out of phase with every cycle.  The infinite loop approach gives you precise control over the frequency, has zero processor overhead, but comes at the expense channel phase imprecision, and not being able to call “rmt_tx_event_callbacks_t::on_trans_done” or “rmt_sync_reset” because the transmission never completes.  

### Limitations - Single shot approach
Setting .loop_count=0 results in single-shot pulses being sent to each respective channel. The transmit calls need to be followed by a rmt_sync_reset(), and that all needs to be to be encapsulated in an infinite while(1){... loop.  The synchronisation is perfect, and the pulse sequences can have arbitrary different lengths.  You can also use callbacks at the end of each sequence to do something.  But, that all comes at the expense of processor cycles, and a loss in the ability to control the precise frequency. 

### compiling code
Install the Espressif IDE, and then use the ESP-IDF command-line shell create a new project.
```
c:> idf.py create-project my_pulse_project
``` 
Copy the c++ file into main/my_pulse_project.c or whatever you called it.  Next, figure out what COM port your device is using.  I find the arduino IDE helpful for that purpose.  Now set the build target, and then compile, and flash.
```
c:> idf.py set-target esp32c3
c:> idf.py build
c:> idf.py flash -p COM12
