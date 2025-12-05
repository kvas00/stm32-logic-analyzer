#include "Tasks.h"
#include "main.h"
#include <cstdio>
#include <stdio.h>

// External logging function
extern void Log_Printf(const char* format, ...);

// External startup banner function
extern void Print_Startup_Banner(void);

// External RTC handle
extern RTC_HandleTypeDef hrtc;

// Logic analyzer test data - Extended for 4 screens (~480 pixels total)
// Format: bit 7 = signal value (0/1), bits 6-0 = time delta in pixels
// Display width ~120 pixels per screen, total 4 screens = 480 pixels
// Random transitions to demonstrate scrolling capability

// CH0: Complex pattern with various pulse widths
const uint8_t logic_ch0_data[] = {
    0x80, 0x14,  // Start LOW for 20 pixels
    0x88, 0x1E,  // Go HIGH for 30 pixels
    0x08, 0x0A,  // Go LOW for 10 pixels
    0x88, 0x19,  // Go HIGH for 25 pixels
    0x08, 0x28,  // Go LOW for 40 pixels
    0x88, 0x0F,  // Go HIGH for 15 pixels
    0x08, 0x14,  // Go LOW for 20 pixels
    0x88, 0x23,  // Go HIGH for 35 pixels
    0x08, 0x0C,  // Go LOW for 12 pixels
    0x88, 0x32,  // Go HIGH for 50 pixels
    0x08, 0x19,  // Go LOW for 25 pixels
    0x88, 0x0A,  // Go HIGH for 10 pixels
    0x08, 0x37,  // Go LOW for 55 pixels
    0x88, 0x14,  // Go HIGH for 20 pixels
    0x08, 0x1E,  // Go LOW for 30 pixels
    0x88, 0x28,  // Go HIGH for 40 pixels
    0x08, 0x0F,  // Go LOW for 15 pixels
    0x88, 0x19,  // Go HIGH for 25 pixels
    0x08, 0x23   // Go LOW for 35 pixels
};

// CH1: Shifted by 3 pixels with different pattern
const uint8_t logic_ch1_data[] = {
    0x80, 0x17,  // Start LOW for 23 pixels (20+3 shift)
    0x88, 0x0F,  // Go HIGH for 15 pixels
    0x08, 0x28,  // Go LOW for 40 pixels
    0x88, 0x14,  // Go HIGH for 20 pixels
    0x08, 0x19,  // Go LOW for 25 pixels
    0x88, 0x32,  // Go HIGH for 50 pixels
    0x08, 0x0A,  // Go LOW for 10 pixels
    0x88, 0x1E,  // Go HIGH for 30 pixels
    0x08, 0x23,  // Go LOW for 35 pixels
    0x88, 0x0C,  // Go HIGH for 12 pixels
    0x08, 0x37,  // Go LOW for 55 pixels
    0x88, 0x19,  // Go HIGH for 25 pixels
    0x08, 0x14,  // Go LOW for 20 pixels
    0x88, 0x28,  // Go HIGH for 40 pixels
    0x08, 0x0F,  // Go LOW for 15 pixels
    0x88, 0x23,  // Go HIGH for 35 pixels
    0x08, 0x1E   // Go LOW for 30 pixels
};

// CH2: Shifted by 6 pixels with another pattern
const uint8_t logic_ch2_data[] = {
    0x80, 0x1A,  // Start LOW for 26 pixels (20+6 shift)
    0x88, 0x23,  // Go HIGH for 35 pixels
    0x08, 0x14,  // Go LOW for 20 pixels
    0x88, 0x0A,  // Go HIGH for 10 pixels
    0x08, 0x32,  // Go LOW for 50 pixels
    0x88, 0x19,  // Go HIGH for 25 pixels
    0x08, 0x0F,  // Go LOW for 15 pixels
    0x88, 0x37,  // Go HIGH for 55 pixels
    0x08, 0x0C,  // Go LOW for 12 pixels
    0x88, 0x1E,  // Go HIGH for 30 pixels
    0x08, 0x28,  // Go LOW for 40 pixels
    0x88, 0x14,  // Go HIGH for 20 pixels
    0x08, 0x19,  // Go LOW for 25 pixels
    0x88, 0x0F,  // Go HIGH for 15 pixels
    0x08, 0x23,  // Go LOW for 35 pixels
    0x88, 0x28   // Go HIGH for 40 pixels
};

// CH3: Shifted by 9 pixels with unique pattern
const uint8_t logic_ch3_data[] = {
    0x80, 0x1D,  // Start LOW for 29 pixels (20+9 shift)
    0x88, 0x28,  // Go HIGH for 40 pixels
    0x08, 0x19,  // Go LOW for 25 pixels
    0x88, 0x0F,  // Go HIGH for 15 pixels
    0x08, 0x23,  // Go LOW for 35 pixels
    0x88, 0x14,  // Go HIGH for 20 pixels
    0x08, 0x32,  // Go LOW for 50 pixels
    0x88, 0x0A,  // Go HIGH for 10 pixels
    0x08, 0x1E,  // Go LOW for 30 pixels
    0x88, 0x37,  // Go HIGH for 55 pixels
    0x08, 0x0C,  // Go LOW for 12 pixels
    0x88, 0x19,  // Go HIGH for 25 pixels
    0x08, 0x28,  // Go LOW for 40 pixels
    0x88, 0x14,  // Go HIGH for 20 pixels
    0x08, 0x0F,  // Go LOW for 15 pixels
    0x88, 0x23   // Go HIGH for 35 pixels
};

// Helper function to calculate total signal length in pixels
static uint16_t calculateSignalLength(const uint8_t* signal_data, uint16_t data_length) {
    uint16_t total_length = 0;
    for (uint16_t i = 0; i < data_length; i++) {
        uint8_t delta = signal_data[i] & 0x7F;  // Extract bits 6-0
        total_length += delta;
    }
    return total_length;
}

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
 * @brief Test Task - Encoder testing and logic analyzer display with scrolling
 *
 * Monitors encoder state and logs events:
 * - Button press/release
 * - Long press
 * - Rotation (CW/CCW with delta and position) - controls horizontal scroll
 * - Prints startup banner after button press or 5 sec timeout (for USB enumeration)
 * - After banner (4 seconds), shows logic analyzer display permanently
 * - Encoder rotation scrolls the display horizontally
 */
void testTask(void* argument) {
    (void)argument;

    static bool last_button_state = true;
    static bool last_long_press = false;
    static bool startup_banner_printed = false;
    static bool logic_analyzer_shown = false;
    static uint32_t banner_time = 0;
    static uint32_t startup_time = 0;
    static uint16_t scroll_offset = 0;  // Horizontal scroll position
    static uint16_t max_scroll = 0;     // Maximum scroll value
    static bool display_needs_update = false;

    // Zoom mode variables
    static bool zoom_mode = false;      // True when in zoom adjustment mode
    static float zoom_level = 1.0f;     // Current zoom level (0.5x, 1.0x, 2.0x, 4.0x, 8.0x)
    static const float zoom_levels[] = {0.5f, 1.0f, 2.0f, 4.0f, 8.0f};
    static const uint8_t num_zoom_levels = sizeof(zoom_levels) / sizeof(zoom_levels[0]);
    static uint8_t current_zoom_index = 1;  // Start at 1.0x (index 1)

    // Screen saver variables
    static uint32_t last_activity_time = 0;  // Last encoder activity timestamp
    static bool display_is_on = true;         // Display power state
    static const uint32_t SCREENSAVER_TIMEOUT = 120000;  // 2 minutes in milliseconds

    // Get startup time
    startup_time = HAL_GetTick();
    last_activity_time = HAL_GetTick();  // Initialize activity time

    // Calculate maximum scroll (once)
    // Max scroll = total signal length - visible width (120 pixels)
    // Use CH0 as reference (all channels should be similar length)
    uint16_t total_signal_length = calculateSignalLength(logic_ch0_data, sizeof(logic_ch0_data));
    uint16_t visible_width = 120;  // ~120 pixels visible area after label
    if (total_signal_length > visible_width) {
        max_scroll = total_signal_length - visible_width;
    } else {
        max_scroll = 0;  // No scrolling needed if signal fits on screen
    }

    for(;;) {
        if (g_encoder != nullptr) {
            // Update encoder state
            g_encoder->update();

            // === TEST MODE ===
            // If TEST_BTN was pressed at startup, enter test mode
            if (g_test_mode) {
                static int last_position = 0;

                int current_position = g_encoder->getPosition();

                // Display test mode info on OLED
                if (g_oled != nullptr) {
                    char buffer[32];

                    // Update display if position changed
                    if (current_position != last_position) {
                        g_oled->clear();

                        // Title
                        g_oled->drawString(0, 0, "***TEST***", 2);

                        // Position
                        snprintf(buffer, sizeof(buffer), "Position: %d", current_position);
                        g_oled->drawString(0, 32, buffer, 1);

                        g_oled->update();

                        last_position = current_position;
                    }
                }

                // Log encoder changes
                int delta = g_encoder->getDelta();
                if (delta != 0) {
                    Log_Printf("[TEST] Pos: %d, Delta: %d\r\n", current_position, delta);
                }

                // Run at 100Hz in test mode
                vTaskDelay(pdMS_TO_TICKS(10));
                continue;  // Skip normal logic analyzer mode
            }
            // === END TEST MODE ===

            // Check for any encoder activity (button or rotation)
            int delta = g_encoder->getDelta();
            bool button_pressed = g_encoder->isButtonPressed();

            // Detect activity: rotation or button press
            if (delta != 0 || (button_pressed && button_pressed != last_button_state)) {
                last_activity_time = HAL_GetTick();  // Update activity timestamp

                // Wake up display if it was off
                if (!display_is_on && g_oled != nullptr && logic_analyzer_shown) {
                    g_oled->displayOn();
                    display_is_on = true;
                    display_needs_update = true;  // Force display refresh
                    Log_Printf("Display ON (wake up)\r\n");
                }
            }

            // Check for screen saver timeout (only after logic analyzer is shown)
            if (logic_analyzer_shown && display_is_on) {
                uint32_t idle_time = HAL_GetTick() - last_activity_time;
                if (idle_time >= SCREENSAVER_TIMEOUT) {
                    if (g_oled != nullptr) {
                        g_oled->displayOff();
                        display_is_on = false;
                        Log_Printf("Display OFF (screen saver after %lu ms idle)\r\n", idle_time);
                    }
                }
            }

            // Print startup banner after button press OR 3 sec timeout
            // This gives USB CDC time to enumerate
            if (!startup_banner_printed) {
                // Note: button_pressed already declared above for activity detection
                uint32_t elapsed = HAL_GetTick() - startup_time;

                if (button_pressed || elapsed >= 3000) {
                    Print_Startup_Banner();
                    startup_banner_printed = true;
                    banner_time = HAL_GetTick();
                }
            }

            // Show logic analyzer display 3 seconds after banner, OR immediately if button pressed
            if (startup_banner_printed && !logic_analyzer_shown) {
                // Note: button_pressed already declared above for activity detection
                uint32_t elapsed_since_banner = HAL_GetTick() - banner_time;

                if (button_pressed || (elapsed_since_banner >= 3000 && g_oled != nullptr)) {
                    logic_analyzer_shown = true;
                    display_needs_update = true;  // Force first display update
                    Log_Printf("Logic analyzer display enabled. Total length: %d px, Max scroll: %d px\r\n",
                              total_signal_length, max_scroll);
                }
            }

            // Handle button press/release
            // Note: button_pressed already declared above for activity detection

            if (button_pressed != last_button_state) {
                if (button_pressed) {
                    // Short press exits zoom mode
                    if (zoom_mode && logic_analyzer_shown) {
                        zoom_mode = false;
                        Log_Printf("Zoom mode OFF (zoom=%.1fx)\r\n", zoom_level);
                        display_needs_update = true;  // Update display to show normal mode
                    } else {
                        Log_Printf("Enter button pressed\r\n");
                    }
                } else {
                    Log_Printf("Enter button released\r\n");
                }
                last_button_state = button_pressed;
            }

            // Handle long press - enters zoom mode (after logic analyzer is shown)
            if (g_encoder->isLongPress() && !last_long_press) {
                last_long_press = true;
                if (logic_analyzer_shown && !zoom_mode) {
                    zoom_mode = true;
                    Log_Printf("Zoom mode ON (zoom=%.1fx) - rotate to adjust, press to exit\r\n", zoom_level);
                    display_needs_update = true;
                } else {
                    Log_Printf("Enter button long press detected\r\n");
                    Log_Printf("Current pos: %d (reset to 0)\r\n", g_encoder->getPosition());
                }
            }
            if (!g_encoder->isLongPress()) {
                last_long_press = false;
            }

            // Handle encoder rotation (after LA is shown)
            // Note: delta already declared above for activity detection
            int pos = g_encoder->getPosition();

            if (delta != 0 && logic_analyzer_shown) {
                if (zoom_mode) {
                    // ZOOM MODE: Adjust zoom level
                    // CW rotation = zoom in (increase zoom)
                    // CCW rotation = zoom out (decrease zoom)
                    int8_t new_zoom_index = current_zoom_index;

                    if (delta > 0) {
                        // Zoom in - increase index (up to max)
                        if (new_zoom_index < (num_zoom_levels - 1)) {
                            new_zoom_index++;
                        }
                    } else {
                        // Zoom out - decrease index (down to 0)
                        if (new_zoom_index > 0) {
                            new_zoom_index--;
                        }
                    }

                    // Update zoom if changed
                    if (new_zoom_index != current_zoom_index) {
                        current_zoom_index = new_zoom_index;
                        zoom_level = zoom_levels[current_zoom_index];
                        display_needs_update = true;

                        // Recalculate max_scroll based on new zoom
                        uint16_t zoomed_signal_length = (uint16_t)(total_signal_length * zoom_level);
                        if (zoomed_signal_length > visible_width) {
                            max_scroll = zoomed_signal_length - visible_width;
                        } else {
                            max_scroll = 0;
                        }

                        // Clamp scroll offset to new max
                        if (scroll_offset > max_scroll) {
                            scroll_offset = max_scroll;
                        }

                        if (delta > 0) {
                            Log_Printf("Zoom IN: %.1fx (max_scroll=%d)\r\n", zoom_level, max_scroll);
                        } else {
                            Log_Printf("Zoom OUT: %.1fx (max_scroll=%d)\r\n", zoom_level, max_scroll);
                        }
                    }
                } else {
                    // NORMAL MODE: Scroll horizontally
                    // CW rotation = scroll right (show left part of signal)
                    // CCW rotation = scroll left (show right part of signal)
                    int16_t new_offset = scroll_offset + (delta * 4);  // 4 pixels per encoder click

                    // Clamp to valid range: [0, max_scroll]
                    if (new_offset < 0) {
                        new_offset = 0;
                    } else if (new_offset > max_scroll) {
                        new_offset = max_scroll;
                    }

                    // Only update if offset actually changed
                    if (new_offset != scroll_offset) {
                        scroll_offset = new_offset;
                        display_needs_update = true;  // Mark display for update

                        if (delta > 0) {
                            Log_Printf("Scroll right: offset=%d, pos=%d (CW, delta=%d)\r\n", scroll_offset, pos, delta);
                        } else {
                            Log_Printf("Scroll left: offset=%d, pos=%d (CCW, delta=%d)\r\n", scroll_offset, pos, delta);
                        }
                    }
                }
            }

            // Update display ONLY when needed (offset changed or first display) AND display is on
            if (logic_analyzer_shown && g_oled != nullptr && display_needs_update && display_is_on) {
                // Clear display
                g_oled->clear();

                // Draw mode indicator in top-left corner
                if (zoom_mode) {
                    // Show "ZOOM" and current zoom level
                    char zoom_str[12];
                    snprintf(zoom_str, sizeof(zoom_str), "Z:%.1fx", zoom_level);
                    g_oled->drawString(0, 0, zoom_str, 1);
                } else {
                    // Show "NORM" in normal scrolling mode
                    g_oled->drawString(0, 0, "NORM", 1);
                }

                // Prepare channel data for drawing
                const uint8_t* channel_data[4] = {
                    logic_ch0_data,
                    logic_ch1_data,
                    logic_ch2_data,
                    logic_ch3_data
                };

                const uint16_t data_lengths[4] = {
                    sizeof(logic_ch0_data),
                    sizeof(logic_ch1_data),
                    sizeof(logic_ch2_data),
                    sizeof(logic_ch3_data)
                };

                // Draw all 4 channels with current scroll offset and zoom level
                g_oled->drawLogicChannels(channel_data, data_lengths, 4, 0, 16, scroll_offset, zoom_level, 1);

                // Update display
                g_oled->update();

                // Clear update flag
                display_needs_update = false;
            }
        }

        // Run at 100Hz (10ms period)
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
