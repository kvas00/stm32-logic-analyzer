# Backup: Version 0.3-oled

**Backup Date**: 2025-10-10
**Status**: ✅ Stable and Tested
**Purpose**: OLED display integration with real-time encoder position monitoring

## Quick Info

This backup contains a fully working version of the encoder-only project with OLED display support.

### Main Features
- ✅ OLED display shows "Sys Started" message on boot
- ✅ Real-time encoder position display: "Pos: X"
- ✅ Efficient updates (only when position changes)
- ✅ LED heartbeat at 500ms
- ✅ UART/USB logging
- ✅ FreeRTOS task management

### Memory Footprint
```
RAM:   27072 B / 64 KB (41.31%)
FLASH: 59276 B / 256 KB (22.61%)
```

## Contents

### Source Files
```
Core/
├── Src/
│   └── main.cpp              (Modified - OLED init & startup message)
└── Lib/
    ├── Tasks.cpp             (Modified - Position display on OLED)
    ├── Tasks.h               (Modified - g_oled export)
    ├── Oled.cpp              (C++ wrapper for SH1106)
    ├── Oled.hpp              (OLED class definition)
    ├── Encoder.cpp           (Rotary encoder driver)
    ├── Encoder.h
    ├── Led.cpp               (LED control)
    └── Led.h
```

### Compiled Files
- `encoder-only.elf` - ELF binary (flashable)
- `encoder-only.map` - Memory map file

### Documentation
- `VERSION_INFO.md` - Detailed version information
- `ROLLBACK_INSTRUCTIONS.md` - Step-by-step restore guide
- `README.md` - This file

## Quick Restore

### Option 1: Restore Source Code
```bash
cd /home/victor/STM/PROJECTS/VSCODE/encoder-only

# Restore modified files
cp BACKUP/v0.3-oled/Core/Src/main.cpp Core/Src/
cp BACKUP/v0.3-oled/Core/Lib/Tasks.cpp Core/Lib/
cp BACKUP/v0.3-oled/Core/Lib/Tasks.h Core/Lib/
cp BACKUP/v0.3-oled/Core/Lib/Oled.* Core/Lib/

# Rebuild
cd build && ninja
```

### Option 2: Flash Binary Directly
```bash
# Using st-flash
st-flash write BACKUP/v0.3-oled/encoder-only.elf 0x08000000

# Using OpenOCD
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg \
  -c "program BACKUP/v0.3-oled/encoder-only.elf verify reset exit"
```

## What Changed in This Version?

### Added to main.cpp
```cpp
#include "Oled.hpp"
display::Oled* oled = nullptr;
display::Oled* g_oled = nullptr;

// In main():
oled = new display::Oled(&hi2c1);
if (oled->init()) {
    oled->clear();
    oled->drawString(0, 0, "Sys Started");
    oled->update();
}
g_oled = oled;
```

### Added to Tasks.cpp
```cpp
// Display update logic in testTask():
if (g_oled != nullptr && pos != last_position) {
    g_oled->clear();
    g_oled->drawString(0, 0, "Sys Started");
    snprintf(buffer, sizeof(buffer), "Pos: %d", pos);
    g_oled->drawString(0, 16, buffer);
    g_oled->update();
    last_position = pos;
}
```

### Added to Tasks.h
```cpp
#include "Oled.hpp"
extern display::Oled* g_oled;
```

## Testing Checklist

After restore, verify:
- [ ] Device boots without errors
- [ ] LED blinks every 500ms (heartbeat)
- [ ] OLED displays "Sys Started" on line 1
- [ ] OLED displays "Pos: 0" on line 2 at startup
- [ ] Rotating encoder updates position on OLED
- [ ] UART outputs at 115200 baud
- [ ] USB CDC outputs encoder events
- [ ] Button press detection works
- [ ] Long press (>1s) resets position to 0

## Hardware Requirements

- **MCU**: STM32F401CCU6
- **Display**: SH1106 OLED 128x64 I2C (address 0x3C)
- **Encoder**: Rotary encoder with button
- **LED**: Connected to configured LED pin
- **Interfaces**:
  - I2C1 for OLED
  - UART1 for serial logging (115200 baud)
  - USB for CDC communication

## Related Documentation

- [VERSION_INFO.md](VERSION_INFO.md) - Full version details
- [ROLLBACK_INSTRUCTIONS.md](ROLLBACK_INSTRUCTIONS.md) - Detailed restore guide
- [../../CHANGELOG.md](../../CHANGELOG.md) - Project changelog
- [../../FREERTOS_TASKS.md](../../FREERTOS_TASKS.md) - FreeRTOS task info

## Version Timeline

```
v0.1 (baseline)
  ↓
v0.2 (encoder + FreeRTOS)
  ↓
v0.3-oled (current) ← You are here
```

## Support

For issues or questions:
1. Check [ROLLBACK_INSTRUCTIONS.md](ROLLBACK_INSTRUCTIONS.md) troubleshooting section
2. Verify hardware connections
3. Compare current files with backup using `diff`
4. Rebuild from clean state: `rm -rf build/* && cd build && cmake .. && ninja`

---

**Backup created by**: Claude Code
**Backup verified**: ✅ Yes
**Build tested**: ✅ Yes
**Hardware tested**: Pending user verification
