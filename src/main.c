// #include <avr/io.h>
// #include <util/delay.h>
// #include <FreeRTOS.h>
// #include <task.h>
// #include "freertos_tasks.h"

// void vApplicationIdleHook(void) {}  // Required for FreeRTOS

// int main(void) {
//     setup();  // Call your setup function

//     // Start FreeRTOS scheduler
//     vTaskStartScheduler();

//     while (1) {
//         // If scheduler stops, stay here
//     }
// }


// Testing the devcontainer-environment...


#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
    // pin 13 as output
    DDRB |= (1 << PB5);

    while (1)
    {
        // let's toggle pin 13
        PORTB ^= (1 << PB5);
        _delay_ms(1000);
    }

    return 0;
}
