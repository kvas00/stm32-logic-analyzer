#include "Tasks.h"
#include "main.h"
#include <cstdio>

// External logging function
extern void Log_Printf(const char* format, ...);

// Task handles (using CMSIS-RTOS types)
osThreadId_t ledTaskHandle = nullptr;
osThreadId_t testTaskHandle = nullptr;

// Shared resources (defined in main.cpp, declared in Tasks.h)
// No need to define here - they are extern in Tasks.h

/**
 * @brief LED Task - Heartbeat indicator
 *
 * Simply blinks LED to show system is alive.
 * Blinks every 500ms.
 */
void ledTask(void* argument) {
    (void)argument;

    TickType_t lastWakeTime = xTaskGetTickCount();
    const TickType_t frequency = pdMS_TO_TICKS(500);  // 500ms period

    for(;;) {
        if (g_led != nullptr) {
            g_led->toggle();
        }

        vTaskDelayUntil(&lastWakeTime, frequency);
    }
}

/**
 * @brief Test Task - Encoder testing
 *
 * Monitors encoder state and logs events:
 * - Button press/release
 * - Long press
 * - Rotation (CW/CCW with delta and position)
 * - Display position on OLED
 */
void testTask(void* argument) {
    (void)argument;

    static bool last_button_state = true;
    static bool last_long_press = false;
    static int last_position = 0;
    char buffer[32];

    for(;;) {
        if (g_encoder != nullptr) {
            // Update encoder state
            g_encoder->update();

            // Handle button press/release
            bool button_pressed = g_encoder->isButtonPressed();

            if (button_pressed != last_button_state) {
                if (button_pressed) {
                    Log_Printf("[%lu ms] Enter button pressed\r\n", HAL_GetTick());
                } else {
                    Log_Printf("[%lu ms] Enter button released\r\n", HAL_GetTick());
                }
                last_button_state = button_pressed;
            }

            // Handle long press
            if (g_encoder->isLongPress() && !last_long_press) {
                last_long_press = true;
                Log_Printf("[%lu ms] Enter button long press detected\r\n", HAL_GetTick());
                Log_Printf("Current pos: %d (reset to 0)\r\n", g_encoder->getPosition());
            }
            if (!g_encoder->isLongPress()) {
                last_long_press = false;
            }

            // Handle encoder rotation
            int delta = g_encoder->getDelta();
            int pos = g_encoder->getPosition();

            if (delta != 0) {
                if (delta > 0) {
                    // CW rotation
                    Log_Printf("Current pos: %d (CW, delta: %d)\r\n", pos, delta);
                } else {
                    // CCW rotation
                    Log_Printf("Current pos: %d (CCW, delta: %d)\r\n", pos, delta);
                }
            }

            // Update OLED display if position changed
            if (g_oled != nullptr && pos != last_position) {
                g_oled->clear();
                g_oled->drawString(0, 0, "Sys Started");

                snprintf(buffer, sizeof(buffer), "Pos: %d", pos);
                g_oled->drawString(0, 16, buffer);

                g_oled->update();
                last_position = pos;
            }
        }

        // Run at 100Hz (10ms period)
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
