#include <avr/io.h>
#include <util/delay.h>
#include <FreeRTOS.h>
#include <task.h>
#include "freertos_tasks.h"

void vApplicationIdleHook(void) {}  // Required for FreeRTOS

int main(void) {
    setup();  // Call your setup function

    // Start FreeRTOS scheduler
    vTaskStartScheduler();

    while (1) {
        // If scheduler stops, stay here
    }
}
