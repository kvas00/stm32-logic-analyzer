# Backup: Version 0.4-date-time

**Backup Date**: 2025-10-10
**Status**: ✅ Stable and Tested
**Purpose**: RTC calendar with timestamps and channel-specific logging

## Main Features

✅ **RTC Calendar** - External 32.768 kHz LSE crystal
✅ **Timestamp Logging** - Format: `[DD.MM.YY HH:MM:SS] message`
✅ **VERBOSE Control** - Master switch for all logging
✅ **Channel Control** - Separate enable/disable for UART and USB
✅ **OLED Display** - Shows date, time, and encoder position
✅ **Version Display** - Shows firmware version on OLED at startup
✅ **Startup Banner** - Version info in logs at boot

## What's New in This Version

### Version Display
- Shows "Encoder-Only" and "v0.4-date-time" on OLED at startup
- Displays for 2 seconds before switching to date/time view
- Logs firmware version and build timestamp to UART/USB

### Logging Improvements
- Combined timestamp + message in single buffer (atomic transmission)
- No more split messages in logs
- Better reliability for concurrent logging

### Example Startup Output

**OLED (2 seconds):**
```
Encoder-Only
v0.4-date-time
```

**Then switches to:**
```
10.10.25
14:00:03
Pos: 0
```

**Logs (UART/USB):**
```
[10.10.25 14:00:00] =================================
[10.10.25 14:00:00] Firmware: v0.4-date-time
[10.10.25 14:00:00] Build: Oct 10 2025 15:30:45
[10.10.25 14:00:00] System started
[10.10.25 14:00:00] =================================
```

## Memory Usage

```
RAM:   27,080 bytes / 64 KB  (41.32%)
FLASH: 61,020 bytes / 256 KB (23.28%)
```

## Logging Configuration

### Main Controls
```cpp
#define FW_VERSION "v0.4-date-time"
#define VERBOSE 1             // Master switch
#define LOG_UART_ENABLED 1    // UART channel
#define LOG_USB_ENABLED  1    // USB CDC channel
```

### Configuration Matrix
| VERBOSE | UART | USB | Result |
|---------|------|-----|--------|
| 0 | X | X | No logging |
| 1 | 0 | 0 | Logging active, no output |
| 1 | 1 | 0 | UART only |
| 1 | 0 | 1 | USB only |
| 1 | 1 | 1 | Both (default) |

## Quick Restore

```bash
cd /home/victor/STM/PROJECTS/VSCODE/encoder-only

# Restore source files
cp BACKUP/v0.4-date-time/Core/Src/main.cpp Core/Src/
cp BACKUP/v0.4-date-time/Core/Lib/Tasks.* Core/Lib/
cp BACKUP/v0.4-date-time/Core/Lib/Oled.* Core/Lib/

# Rebuild
cd build && ninja

# Or flash binary directly
st-flash write BACKUP/v0.4-date-time/encoder-only.elf 0x08000000
```

## File List

### Source Files
- `Core/Src/main.cpp` - Main with version display and logging
- `Core/Lib/Tasks.cpp` - FreeRTOS tasks with OLED time display
- `Core/Lib/Tasks.h` - Task headers with RTC extern
- `Core/Lib/Oled.cpp/hpp` - OLED driver wrapper
- `Core/Lib/Encoder.cpp/h` - Encoder driver
- `Core/Lib/Led.cpp/h` - LED driver

### Binaries
- `encoder-only.elf` - Flashable firmware
- `encoder-only.map` - Memory map

### Documentation
- `README.md` - This file
- `checksums.sha256` - File integrity checksums

## Key Features

### 1. RTC Calendar
- 24-hour format
- Initial time: 10.10.2025 14:00:00
- External LSE 32.768 kHz crystal
- Auto-increments (seconds, minutes, hours, days, months, years)

### 2. Timestamp Logging
- Format: `[DD.MM.YY HH:MM:SS] <message>`
- Atomic transmission (timestamp + message combined)
- Dual output: UART1 (115200 baud) + USB CDC

### 3. OLED Display
Shows 3 lines updated every second or on encoder change:
- Line 1: Date (DD.MM.YY)
- Line 2: Time (HH:MM:SS)
- Line 3: Encoder position (Pos: X)

### 4. Version Information
- Defined in main.cpp: `FW_VERSION`
- Displayed on OLED at startup (2 sec)
- Logged to UART/USB with build timestamp

### 5. Logging Control
Three-level control:
1. **VERBOSE** - Master switch (priority)
2. **LOG_UART_ENABLED** - UART channel
3. **LOG_USB_ENABLED** - USB channel

## Hardware Requirements

- **MCU**: STM32F401CCU6
- **OLED**: SH1106 128x64 I2C (0x3C)
- **Encoder**: Rotary with button
- **RTC**: External LSE 32.768 kHz crystal
- **UART**: UART1 at 115200 baud
- **USB**: USB FS CDC Virtual COM

## Dependencies

**Included in backup:**
- ✅ All modified source files
- ✅ Compiled binaries
- ✅ Checksums

**Not included (must be present):**
- CMakeLists.txt
- STM32 HAL libraries
- FreeRTOS kernel
- USB Device library
- sh1106.c/h drivers

## Verification

Check file integrity:
```bash
cd BACKUP/v0.4-date-time
sha256sum -c checksums.sha256
```

## Version Timeline

```
v0.1 → v0.2 → v0.3-oled → v0.4-date-time ← You are here
```

## Related Documentation

- [../../CHANGELOG.md](../../CHANGELOG.md) - Full changelog
- [../../RTC_LOGGING_FEATURES.md](../../RTC_LOGGING_FEATURES.md) - RTC details
- [../../LOGGING_CONFIGURATION.md](../../LOGGING_CONFIGURATION.md) - Logging config

---

**Created by**: Claude Code
**Build Date**: 2025-10-10
**Status**: ✅ Verified and Ready
