#include "Tasks.h"
#include "main.h"
#include <cstdio>

// External logging function
extern void Log_Printf(const char* format, ...);

// External RTC handle
extern RTC_HandleTypeDef hrtc;

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
 * @brief Test Task - Encoder testing and display update
 *
 * Monitors encoder state and logs events:
 * - Button press/release
 * - Long press
 * - Rotation (CW/CCW with delta and position)
 * - Display RTC time/date and position on OLED (updated every second)
 */
void testTask(void* argument) {
    (void)argument;

    static bool last_button_state = true;
    static bool last_long_press = false;
    static int last_position = 0;
    static uint32_t last_display_update = 0;
    char buffer[32];

    for(;;) {
        if (g_encoder != nullptr) {
            // Update encoder state
            g_encoder->update();

            // Handle button press/release
            bool button_pressed = g_encoder->isButtonPressed();

            if (button_pressed != last_button_state) {
                if (button_pressed) {
                    Log_Printf("Enter button pressed\r\n");
                } else {
                    Log_Printf("Enter button released\r\n");
                }
                last_button_state = button_pressed;
            }

            // Handle long press
            if (g_encoder->isLongPress() && !last_long_press) {
                last_long_press = true;
                Log_Printf("Enter button long press detected\r\n");
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

            // Update OLED display every second OR if position changed
            uint32_t current_tick = HAL_GetTick();
            bool position_changed = (pos != last_position);
            bool time_to_update = (current_tick - last_display_update >= 1000);

            if (g_oled != nullptr && (position_changed || time_to_update)) {
                // Get current time from RTC
                RTC_TimeTypeDef sTime;
                RTC_DateTypeDef sDate;
                HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
                HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

                // Clear display
                g_oled->clear();

                // Line 1: Date (DD.MM.YY)
                snprintf(buffer, sizeof(buffer), "%02d.%02d.%02d",
                         sDate.Date, sDate.Month, sDate.Year);
                g_oled->drawString(0, 0, buffer);

                // Line 2: Time (HH:MM:SS)
                snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d",
                         sTime.Hours, sTime.Minutes, sTime.Seconds);
                g_oled->drawString(0, 12, buffer);

                // Line 3: Encoder position
                snprintf(buffer, sizeof(buffer), "Pos: %d", pos);
                g_oled->drawString(0, 28, buffer);

                // Update display
                g_oled->update();

                last_position = pos;
                last_display_update = current_tick;
            }
        }

        // Run at 100Hz (10ms period)
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
