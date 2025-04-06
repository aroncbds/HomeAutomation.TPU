#include <Arduino.h>
#include "tpu_firmware.h"

int main(void) {
    init();         // Arduino-specific init (sets up timers, serial, etc.)
    setup();        // your sketch's setup

    while (1) {
        loop();     // your sketch's loop
    }

    return 0;
}
