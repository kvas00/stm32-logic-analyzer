# Version 0.3-oled

**Date**: 2025-10-10
**Status**: Stable
**Build**: Release

## Overview

This version adds OLED display support with real-time encoder position monitoring.
The system displays startup message and continuously updates the current encoder position on the SH1106 OLED display.

## Features Added

### 1. OLED Display Integration
- **Display Controller**: SH1106 (128x64 pixels)
- **Interface**: I2C1
- **C++ Wrapper**: `display::Oled` class wrapping low-level SH1106 driver

### 2. Startup Message
- Displays "Sys Started" on OLED at system initialization
- Shows on line 1 (y=0) of the display

### 3. Real-time Encoder Position Display
- Continuously monitors encoder position
- Updates OLED display when position changes
- Format: "Pos: X" on line 2 (y=16)
- Update rate: Only when position changes (efficient)

## Modified Files

### Core/Src/main.cpp
- Added `#include "Oled.hpp"`
- Added global OLED object: `display::Oled* oled`
- Added exported pointer: `display::Oled* g_oled`
- Initialized OLED in main() after I2C initialization
- Display startup message "Sys Started"

### Core/Lib/Tasks.cpp
- Modified `testTask()` to update OLED display
- Added position tracking: `static int last_position`
- Added display update on position change
- Shows both "Sys Started" and current position

### Core/Lib/Tasks.h
- Added `#include "Oled.hpp"`
- Added extern declaration: `extern display::Oled* g_oled`

### Core/Lib/Oled.cpp & Oled.hpp
- C++ wrapper class for SH1106 OLED display
- Methods: init(), clear(), update(), drawString(), drawChar()
- Hardware abstraction over low-level C driver

## Memory Usage

```
Memory region         Used Size  Region Size  %age Used
             RAM:       27072 B        64 KB     41.31%
           FLASH:       59276 B       256 KB     22.61%
```

## Hardware Configuration

### OLED Display
- **I2C Address**: 0x3C (SH1106_I2C_ADDR)
- **Resolution**: 128x64 pixels
- **Interface**: I2C1 at 100kHz
- **Pins**:
  - SDA: I2C1_SDA
  - SCL: I2C1_SCL

### Encoder
- **Channel A**: ENCODER_A_Pin (EXTI interrupt)
- **Channel B**: ENCODER_B_Pin (GPIO input)
- **Button**: ENCODER_ENTER_Pin (GPIO input)
- **Pull-up**: Internal pull-ups enabled

## Software Architecture

### Task Structure
1. **defaultTask** (Normal priority, 256*4 stack)
   - USB CDC initialization

2. **ledTask** (Low priority, 512*4 stack)
   - LED heartbeat (500ms blink)

3. **testTask** (Normal priority, 256*4 stack)
   - Encoder monitoring
   - OLED display update
   - UART/USB logging
   - 100Hz update rate (10ms period)

### Display Update Logic
```cpp
if (g_oled != nullptr && pos != last_position) {
    g_oled->clear();
    g_oled->drawString(0, 0, "Sys Started");
    snprintf(buffer, sizeof(buffer), "Pos: %d", pos);
    g_oled->drawString(0, 16, buffer);
    g_oled->update();
    last_position = pos;
}
```

## Build Information

- **Build System**: CMake + Ninja
- **Compiler**: arm-none-eabi-gcc 14.2.0
- **Target**: STM32F401CCU6
- **RTOS**: FreeRTOS (CMSIS-RTOS v2 API)
- **HAL**: STM32 HAL
- **C++ Standard**: C++17

## Testing Notes

### Expected Behavior
1. On power-up/reset:
   - LED starts blinking (heartbeat)
   - OLED displays "Sys Started"
   - Initial encoder position displayed as "Pos: 0"

2. When encoder is rotated:
   - Position updates on OLED in real-time
   - UART/USB logs rotation events
   - Display shows current position value

3. When encoder button is pressed:
   - UART/USB logs button events
   - Long press (>1s) resets position to 0
   - Display updates to show new position

### Verification Checklist
- [ ] OLED displays startup message
- [ ] Encoder position updates on display
- [ ] LED blinks at 500ms interval
- [ ] UART logs encoder events at 115200 baud
- [ ] USB CDC logs encoder events
- [ ] Button press detection works
- [ ] Long press resets position
- [ ] Display updates only when position changes

## Dependencies

### Hardware Dependencies
- SH1106 OLED display connected to I2C1
- Rotary encoder with button
- USB connection for CDC communication
- UART1 for serial logging (115200 baud)

### Software Dependencies
- `sh1106.c/h` - Low-level SH1106 driver
- `sh1106_font.c` - Font definitions
- FreeRTOS kernel
- STM32 HAL library
- USB Device library (CDC class)

## Known Issues
None

## Future Improvements
- Add multiple display screens/menus
- Implement menu navigation using encoder
- Add graphics/icons to display
- Display system information (uptime, button state)
- Add display contrast adjustment
- Implement screensaver

## Version History
- v0.1: Initial project setup
- v0.2: Encoder and LED integration
- v0.3-oled: **Current version** - OLED display with position monitoring
