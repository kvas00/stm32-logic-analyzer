# Logging Configuration Guide

**Version**: 0.5-logging-channels
**Date**: 2025-10-10

## Overview

The project supports flexible logging configuration with three control defines:
- `VERBOSE` - Master switch for all logging
- `LOG_UART_ENABLED` - Enable/disable UART channel
- `LOG_USB_ENABLED` - Enable/disable USB CDC channel

## Control Defines

### Location
All defines are in [Core/Src/main.cpp](Core/Src/main.cpp) lines 46-52:

```cpp
// Verbose logging control (1 = enable all logging, 0 = disable all logging)
// VERBOSE has priority over LOG_USB_ENABLED and LOG_UART_ENABLED
#define VERBOSE 1

// Individual channel control (only works if VERBOSE = 1)
#define LOG_UART_ENABLED 1  // Enable/disable UART logging
#define LOG_USB_ENABLED  1  // Enable/disable USB CDC logging
```

## Priority Rules

**VERBOSE has priority over channel-specific defines:**

1. If `VERBOSE = 0`: **ALL logging is disabled** (regardless of channel settings)
2. If `VERBOSE = 1`: Logging is enabled, channels controlled individually

### Priority Matrix

| VERBOSE | LOG_UART_ENABLED | LOG_USB_ENABLED | Result |
|---------|------------------|-----------------|---------|
| 0 | 0 | 0 | ❌ No logging anywhere |
| 0 | 0 | 1 | ❌ No logging anywhere |
| 0 | 1 | 0 | ❌ No logging anywhere |
| 0 | 1 | 1 | ❌ No logging anywhere |
| 1 | 0 | 0 | ⚠️ Logging code runs but no output |
| 1 | 0 | 1 | ✅ Only USB CDC output |
| 1 | 1 | 0 | ✅ Only UART output |
| 1 | 1 | 1 | ✅ Both UART and USB output |

## Configuration Examples

### Example 1: All Logging Enabled (Default)
```cpp
#define VERBOSE 1
#define LOG_UART_ENABLED 1
#define LOG_USB_ENABLED  1
```
**Result**: Logs to both UART1 (115200 baud) and USB CDC
**Use case**: Development, debugging, full logging

### Example 2: USB Only
```cpp
#define VERBOSE 1
#define LOG_UART_ENABLED 0
#define LOG_USB_ENABLED  1
```
**Result**: Logs only to USB CDC
**Use case**: PC connected via USB, no UART cable needed

### Example 3: UART Only
```cpp
#define VERBOSE 1
#define LOG_UART_ENABLED 1
#define LOG_USB_ENABLED  0
```
**Result**: Logs only to UART1
**Use case**: Serial terminal debugging, no USB dependency

### Example 4: All Logging Disabled
```cpp
#define VERBOSE 0
#define LOG_UART_ENABLED 1  // Ignored
#define LOG_USB_ENABLED  1  // Ignored
```
**Result**: No logging at all (maximum code optimization)
**Use case**: Production build, minimize code size and CPU usage

### Example 5: Logging Enabled But No Output
```cpp
#define VERBOSE 1
#define LOG_UART_ENABLED 0
#define LOG_USB_ENABLED  0
```
**Result**: Logging code runs (RTC read, formatting) but no output
**Use case**: Testing RTC performance without I/O overhead

## Output Format

When logging is enabled, all messages are prefixed with timestamp:

```
[DD.MM.YY HH:MM:SS] <your message>
```

**Example output**:
```
[10.10.25 14:00:03] Enter button pressed
[10.10.25 14:00:05] Current pos: 5 (CW, delta: 1)
[10.10.25 14:00:08] Enter button released
```

## Memory Impact

### Flash Memory Usage

| Configuration | FLASH Used | Savings vs Full |
|---------------|------------|-----------------|
| VERBOSE=1, Both channels | 60,824 B | 0 B (baseline) |
| VERBOSE=1, UART only | ~60,700 B | ~124 B |
| VERBOSE=1, USB only | ~60,700 B | ~124 B |
| VERBOSE=1, No channels | ~60,600 B | ~224 B |
| VERBOSE=0 (all disabled) | ~59,500 B | ~1,324 B |

### RAM Usage

| Configuration | RAM Impact |
|---------------|-----------|
| VERBOSE=1 | 288 bytes stack per Log_Printf() call |
| VERBOSE=0 | 0 bytes (function is empty) |

**Note**: Stack usage is temporary and released after function returns

## Performance Impact

### Typical Log_Printf() Call Duration

| Operation | Time (µs) | Notes |
|-----------|-----------|-------|
| RTC read | 20-30 | Read time + date |
| Timestamp format | 50-100 | snprintf() |
| Message format | 50-150 | snprintf() with args |
| UART transmit | 200-400 | @115200 baud, ~50 chars |
| USB CDC transmit | 50-150 | USB FS speed |
| **Total (both)** | **370-830** | Full logging |
| **Total (USB only)** | **170-430** | No UART wait |
| **Total (UART only)** | **320-680** | No USB overhead |
| **Total (VERBOSE=0)** | **0** | Compiled out |

### Recommendations

- **Development**: Use both channels for redundancy
- **USB debugging**: Disable UART to reduce transmission time
- **Serial debugging**: Disable USB to reduce overhead
- **Production**: Set VERBOSE=0 for zero overhead

## Hardware Configuration

### UART Channel
- **Interface**: UART1
- **Baud rate**: 115200
- **Data bits**: 8
- **Stop bits**: 1
- **Parity**: None
- **Pins**: TX (UART1_TX), RX (UART1_RX)

### USB CDC Channel
- **Interface**: USB FS (Full Speed)
- **Class**: CDC ACM (Virtual COM Port)
- **Enumeration**: Automatic on USB connection
- **Buffer**: USB FS TX buffer

## Code Implementation

### Log_Printf Function

Location: [Core/Src/main.cpp:114-153](Core/Src/main.cpp#L114-L153)

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
#if LOG_UART_ENABLED
    // Send to UART1 if enabled
    HAL_UART_Transmit(&huart1, (uint8_t*)timestamp, strlen(timestamp), 100);
    HAL_UART_Transmit(&huart1, (uint8_t*)buffer, len, 100);
#endif

#if LOG_USB_ENABLED
    // Send to USB-CDC if enabled
    CDC_Transmit_FS((uint8_t*)timestamp, strlen(timestamp));
    CDC_Transmit_FS((uint8_t*)buffer, len);
#endif
  }
#endif
}
```

## Usage in Code

### Basic Logging
```cpp
Log_Printf("System started\r\n");
Log_Printf("Encoder position: %d\r\n", position);
Log_Printf("Button state: %s\r\n", pressed ? "pressed" : "released");
```

### Conditional Logging
If you need runtime conditional logging (in addition to compile-time):

```cpp
bool debug_mode = true;

if (debug_mode) {
    Log_Printf("Debug: variable x = %d\r\n", x);
}
```

### High-Frequency Logging
For high-frequency events, consider rate limiting:

```cpp
static uint32_t last_log_time = 0;
uint32_t now = HAL_GetTick();

if (now - last_log_time >= 1000) {  // Log once per second
    Log_Printf("High-freq event, count: %lu\r\n", event_count);
    last_log_time = now;
}
```

## Best Practices

### 1. Choose Appropriate Configuration

**Development Phase**:
```cpp
#define VERBOSE 1
#define LOG_UART_ENABLED 1
#define LOG_USB_ENABLED  1
```

**Testing Phase**:
```cpp
#define VERBOSE 1
#define LOG_UART_ENABLED 1  // Or just one channel
#define LOG_USB_ENABLED  0
```

**Production Release**:
```cpp
#define VERBOSE 0
#define LOG_UART_ENABLED 0
#define LOG_USB_ENABLED  0
```

### 2. Message Format Guidelines

✅ **Good**:
```cpp
Log_Printf("Encoder CW, pos: %d\r\n", position);
Log_Printf("Button pressed\r\n");
Log_Printf("I2C error: 0x%02X\r\n", error_code);
```

❌ **Avoid**:
```cpp
Log_Printf("This is a very long message with lots of unnecessary details...\r\n");
Log_Printf("%d\r\n", x);  // Too terse, no context
```

### 3. Performance Considerations

- **Avoid logging in ISRs** (Interrupt Service Routines)
- **Rate-limit high-frequency logs** (>10 Hz)
- **Keep messages short** (<80 characters)
- **Use UART only for critical paths** (USB has less overhead)

### 4. Testing Configurations

Always test these scenarios:
1. VERBOSE=1, both channels enabled
2. VERBOSE=1, UART only
3. VERBOSE=1, USB only
4. VERBOSE=0 (all disabled)

## Troubleshooting

### Problem: No output on UART

**Check**:
1. `VERBOSE = 1`
2. `LOG_UART_ENABLED = 1`
3. UART cable connected (TX→RX, GND→GND)
4. Serial terminal configured: 115200, 8N1
5. Correct COM port selected

### Problem: No output on USB CDC

**Check**:
1. `VERBOSE = 1`
2. `LOG_USB_ENABLED = 1`
3. USB cable connected (data cable, not power-only)
4. USB enumeration successful (check Device Manager)
5. Virtual COM port driver installed
6. Terminal connected to correct COM port

### Problem: Garbled output

**Causes**:
- Wrong baud rate (should be 115200)
- Wrong line ending settings
- Buffer overflow (message too long)

**Solutions**:
- Set terminal to 115200, 8N1
- Use `\r\n` line endings
- Keep messages under 256 characters

### Problem: Intermittent output

**Causes**:
- USB CDC buffer full
- UART buffer full
- High logging frequency

**Solutions**:
- Reduce logging frequency
- Add delays between logs
- Increase buffer sizes (modify code)

## Build Workflow

### Quick Configuration Change

1. Edit [Core/Src/main.cpp](Core/Src/main.cpp) lines 48-52
2. Change desired defines
3. Rebuild:
   ```bash
   cd build
   ninja
   ```
4. Flash firmware
5. Test logging output

### Example: Switch to USB-only Logging

```bash
# Edit main.cpp
#define VERBOSE 1
#define LOG_UART_ENABLED 0  # ← Changed from 1
#define LOG_USB_ENABLED  1

# Rebuild
cd build && ninja

# Flash
st-flash write encoder-only.elf 0x08000000
```

## Related Documentation

- [RTC_LOGGING_FEATURES.md](RTC_LOGGING_FEATURES.md) - RTC configuration
- [CHANGELOG.md](CHANGELOG.md) - Version history
- [Core/Src/main.cpp](Core/Src/main.cpp) - Implementation

## Version History

- **v0.5-logging-channels** - Added channel-specific control (current)
- **v0.4-rtc-logging** - Added RTC timestamps and VERBOSE
- **v0.3-oled** - OLED display integration
- **v0.2** - FreeRTOS integration
- **v0.1** - Initial baseline

---

**Last Updated**: 2025-10-10
**Status**: ✅ Tested and Working
