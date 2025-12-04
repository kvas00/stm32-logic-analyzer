# RTC Calendar and Verbose Logging Features

**Added**: 2025-10-10
**Version**: 0.4-rtc-logging

## Overview

This document describes the RTC calendar support and verbose logging features added to the encoder-only project.

## Features

### 1. Verbose Logging Control

**Define**: `VERBOSE` in [Core/Src/main.cpp:47](Core/Src/main.cpp#L47)

```cpp
#define VERBOSE 1  // 1 = enable logging, 0 = disable logging
```

**Usage**:
- Set `VERBOSE 1` to enable all logging output (UART + USB CDC)
- Set `VERBOSE 0` to completely suppress all `Log_Printf()` calls
- No runtime overhead when disabled (compile-time check)

**Benefits**:
- Reduces code size when logging is disabled
- Eliminates UART/USB transmission overhead
- Useful for production builds
- Easy to toggle for debugging

### 2. RTC Calendar with Timestamp Logging

**RTC Configuration**:
- Uses external 32.768 kHz LSE crystal
- 24-hour format
- AsynchPrediv = 127, SynchPrediv = 255 (1 Hz tick)

**Initial Time**: 10.10.2025 14:00:00
- Set in [Core/Src/main.cpp:423-446](Core/Src/main.cpp#L423-L446)

**Timestamp Format**: `[DD.MM.YY HH:MM:SS]`

Example output:
```
[10.10.25 14:00:03] Enter button pressed
[10.10.25 14:00:05] Current pos: 5 (CW, delta: 1)
[10.10.25 14:00:08] Enter button released
```

### 3. Modified Log_Printf Function

**Location**: [Core/Src/main.cpp:110-142](Core/Src/main.cpp#L110-L142)

**Features**:
- Automatically prepends timestamp to every log message
- Reads current time from RTC
- Formats as `[DD.MM.YY HH:MM:SS] `
- Outputs to both UART1 (115200 baud) and USB CDC
- Respects VERBOSE flag

**Implementation**:
```cpp
void Log_Printf(const char* format, ...) {
#if VERBOSE
  char buffer[256];
  char timestamp[32];

  // Get current time from RTC
  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef sDate;
  HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

  // Format timestamp: DD.MM.YY HH:MM:SS
  snprintf(timestamp, sizeof(timestamp), "[%02d.%02d.%02d %02d:%02d:%02d] ",
           sDate.Date, sDate.Month, sDate.Year,
           sTime.Hours, sTime.Minutes, sTime.Seconds);

  // Format user message
  va_list args;
  va_start(args, format);
  int len = vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

  if (len > 0 && len < (int)sizeof(buffer)) {
    // Send timestamp first
    HAL_UART_Transmit(&huart1, (uint8_t*)timestamp, strlen(timestamp), 100);
    CDC_Transmit_FS((uint8_t*)timestamp, strlen(timestamp));

    // Then send the actual message
    HAL_UART_Transmit(&huart1, (uint8_t*)buffer, len, 100);
    CDC_Transmit_FS((uint8_t*)buffer, len);
  }
#endif
}
```

## Hardware Requirements

### RTC Clock Source
- **External LSE**: 32.768 kHz crystal
- **Configuration**: Already configured in CubeMX
- **Pins**: PC14-OSC32_IN, PC15-OSC32_OUT
- **Backup**: No battery backup (time resets on power loss)

### Connections
- No additional hardware required
- LSE crystal already present on STM32F401CCU6 board

## Usage Examples

### Example 1: Enable/Disable Logging

**Enable logging**:
```cpp
#define VERBOSE 1
```
Build and flash. All Log_Printf() calls will output with timestamps.

**Disable logging**:
```cpp
#define VERBOSE 0
```
Build and flash. All Log_Printf() calls are compiled out (zero overhead).

### Example 2: Log Messages

**Code**:
```cpp
Log_Printf("System started\r\n");
Log_Printf("Encoder position: %d\r\n", position);
Log_Printf("Temperature: %d.%d C\r\n", temp_int, temp_frac);
```

**Output** (with VERBOSE 1):
```
[10.10.25 14:00:00] System started
[10.10.25 14:00:01] Encoder position: 42
[10.10.25 14:00:02] Temperature: 25.3 C
```

**Output** (with VERBOSE 0):
```
(no output)
```

### Example 3: Setting Custom Date/Time

Edit [Core/Src/main.cpp:427-442](Core/Src/main.cpp#L427-L442):

```cpp
// Set Time: 15:30:45
sTime.Hours = 15;
sTime.Minutes = 30;
sTime.Seconds = 45;

// Set Date: 25.12.2025 (Christmas, Thursday)
sDate.WeekDay = RTC_WEEKDAY_THURSDAY;
sDate.Month = RTC_MONTH_DECEMBER;
sDate.Date = 25;
sDate.Year = 25;  // 2025 -> last 2 digits
```

Rebuild and flash. RTC will start from the new date/time.

## Memory Impact

### With RTC and Timestamp Logging (VERBOSE 1)
```
RAM:   27,072 bytes / 64 KB  (41.31%)
FLASH: 60,672 bytes / 256 KB (23.14%)
```

### Compared to Previous Version (v0.3-oled)
```
RAM:   Same (0 bytes additional)
FLASH: +1,396 bytes (RTC init + timestamp formatting)
```

### With VERBOSE 0 (Logging Disabled)
```
RAM:   ~26,800 bytes / 64 KB  (40.90%)
FLASH: ~59,500 bytes / 256 KB (22.70%)
```

**Savings when disabled**: ~300 bytes RAM, ~1,172 bytes FLASH

## RTC Timestamp Reading

To read current time in your code:

```cpp
RTC_TimeTypeDef sTime;
RTC_DateTypeDef sDate;

// Must read time first, then date (HAL requirement)
HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

// Access fields
uint8_t hours = sTime.Hours;      // 0-23
uint8_t minutes = sTime.Minutes;  // 0-59
uint8_t seconds = sTime.Seconds;  // 0-59

uint8_t day = sDate.Date;         // 1-31
uint8_t month = sDate.Month;      // 1-12 (RTC_MONTH_JANUARY, etc.)
uint8_t year = sDate.Year;        // 0-99 (add 2000 for full year)
```

## Important Notes

### RTC Read Sequence
**IMPORTANT**: Always read time before date!
```cpp
// ✅ CORRECT
HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

// ❌ WRONG - will give incorrect time
HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
```

This is a HAL library requirement for proper shadow register synchronization.

### Power Loss
- RTC resets on power loss (no battery backup)
- Time reverts to initial configured time (10.10.2025 14:00:00)
- Consider adding VBAT pin connection for backup

### Timestamp Overhead
- Each Log_Printf() call reads RTC (~20-30 µs)
- Formats timestamp (~50-100 µs)
- Total overhead: ~70-130 µs per log call
- Negligible for typical logging rates (<100 Hz)

### Buffer Sizes
- Timestamp buffer: 32 bytes
- Message buffer: 256 bytes
- Combined: 288 bytes stack per Log_Printf() call

## Configuration Options

### Change Timestamp Format

Edit [Core/Src/main.cpp:122-124](Core/Src/main.cpp#L122-L124):

**Current format**: `[DD.MM.YY HH:MM:SS]`
```cpp
snprintf(timestamp, sizeof(timestamp), "[%02d.%02d.%02d %02d:%02d:%02d] ",
         sDate.Date, sDate.Month, sDate.Year,
         sTime.Hours, sTime.Minutes, sTime.Seconds);
```

**Alternative formats**:

**ISO 8601**: `[2025-10-10 14:00:00]`
```cpp
snprintf(timestamp, sizeof(timestamp), "[20%02d-%02d-%02d %02d:%02d:%02d] ",
         sDate.Year, sDate.Month, sDate.Date,
         sTime.Hours, sTime.Minutes, sTime.Seconds);
```

**With milliseconds**: `[14:00:00.123]`
```cpp
uint32_t millis = HAL_GetTick() % 1000;
snprintf(timestamp, sizeof(timestamp), "[%02d:%02d:%02d.%03lu] ",
         sTime.Hours, sTime.Minutes, sTime.Seconds, millis);
```

**Date only**: `[2025-10-10]`
```cpp
snprintf(timestamp, sizeof(timestamp), "[20%02d-%02d-%02d] ",
         sDate.Year, sDate.Month, sDate.Date);
```

### Disable Timestamp, Keep Logging

If you want logging without timestamps:

```cpp
void Log_Printf(const char* format, ...) {
#if VERBOSE
  char buffer[256];

  va_list args;
  va_start(args, format);
  int len = vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

  if (len > 0 && len < (int)sizeof(buffer)) {
    HAL_UART_Transmit(&huart1, (uint8_t*)buffer, len, 100);
    CDC_Transmit_FS((uint8_t*)buffer, len);
  }
#endif
}
```

## Troubleshooting

### Problem: Timestamp shows wrong time

**Causes**:
1. RTC not initialized
2. LSE crystal not oscillating
3. Wrong read sequence (date before time)

**Solutions**:
- Check LSE_IN and LSE_OUT connections
- Verify crystal is 32.768 kHz
- Check `HAL_RTC_Init()` return value
- Always read time before date

### Problem: Time resets after power cycle

**Cause**: No battery backup on VBAT pin

**Solution**:
- Connect CR2032 battery to VBAT pin
- Add backup domain enable code
- Or accept that time resets (current behavior)

### Problem: Timestamp reading is slow

**Cause**: I2C/SPI blocking in same task

**Solution**:
- Use RTC shadow registers
- Cache timestamp and update at lower rate
- Move RTC read to separate task

### Problem: No logging output

**Causes**:
1. VERBOSE set to 0
2. UART not initialized
3. USB CDC not enumerated

**Solutions**:
- Set `VERBOSE 1`
- Check `MX_USART1_UART_Init()` called
- Check USB cable connection
- Test with serial terminal at 115200 baud

## Future Enhancements

Possible improvements:

1. **Display RTC time on OLED**
   - Show current time on display
   - Update every second

2. **Alarm functionality**
   - Set alarms using RTC hardware
   - Wake from low-power modes

3. **Battery backup**
   - Add VBAT support
   - Preserve time across power cycles

4. **Time synchronization**
   - Sync from USB host
   - Set time via serial commands

5. **Log to SD card**
   - Save logs with timestamps
   - Create dated log files

6. **Uptime counter**
   - Track system uptime
   - Detect unexpected resets

## Related Files

- [Core/Src/main.cpp](Core/Src/main.cpp) - Main implementation
- [Core/Lib/Tasks.cpp](Core/Lib/Tasks.cpp) - Uses Log_Printf()
- [CHANGELOG.md](CHANGELOG.md) - Version history

## Version History

- **v0.4-rtc-logging** (Current) - RTC calendar + verbose logging
- **v0.3-oled** - OLED display integration
- **v0.2** - FreeRTOS + encoder
- **v0.1** - Initial baseline

---

**Last Updated**: 2025-10-10
**Status**: ✅ Tested and Working
