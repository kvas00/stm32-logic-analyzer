#ifndef TASKS_H
#define TASKS_H

#include "cmsis_os.h"
#include "Led.h"
#include "Encoder.h"
#include "Oled.hpp"

// Task handles (using CMSIS-RTOS types)
extern osThreadId_t ledTaskHandle;
extern osThreadId_t testTaskHandle;

// Shared resources
extern Led* g_led;
extern Encoder* g_encoder;
extern display::Oled* g_oled;

// Test mode flag (set at startup if TEST_BTN pressed)
extern bool g_test_mode;

// Task functions
void ledTask(void* argument);
void testTask(void* argument);

#endif // TASKS_H
