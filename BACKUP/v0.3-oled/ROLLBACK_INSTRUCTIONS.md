# Rollback Instructions for Version 0.3-oled

This document provides step-by-step instructions for restoring the project to version 0.3-oled state.

## Quick Rollback

If you need to quickly restore this version, execute the following commands from the project root:

```bash
# Navigate to project root
cd /home/victor/STM/PROJECTS/VSCODE/encoder-only

# Restore source files
cp BACKUP/v0.3-oled/Core/Src/main.cpp Core/Src/
cp BACKUP/v0.3-oled/Core/Lib/Tasks.cpp Core/Lib/
cp BACKUP/v0.3-oled/Core/Lib/Tasks.h Core/Lib/
cp BACKUP/v0.3-oled/Core/Lib/Oled.cpp Core/Lib/
cp BACKUP/v0.3-oled/Core/Lib/Oled.hpp Core/Lib/

# Optional: Restore support libraries if needed
cp BACKUP/v0.3-oled/Core/Lib/Encoder.cpp Core/Lib/
cp BACKUP/v0.3-oled/Core/Lib/Encoder.h Core/Lib/
cp BACKUP/v0.3-oled/Core/Lib/Led.cpp Core/Lib/
cp BACKUP/v0.3-oled/Core/Lib/Led.h Core/Lib/

# Rebuild project
cd build
cmake ..
ninja

# Or if using make:
# make clean && make -j$(nproc)
```

## Detailed Rollback Steps

### Step 1: Verify Backup Integrity

Check that all required files exist in the backup:

```bash
ls -la BACKUP/v0.3-oled/Core/Src/
ls -la BACKUP/v0.3-oled/Core/Lib/
```

Expected files:
- `Core/Src/main.cpp`
- `Core/Lib/Tasks.cpp`
- `Core/Lib/Tasks.h`
- `Core/Lib/Oled.cpp`
- `Core/Lib/Oled.hpp`
- `Core/Lib/Encoder.cpp`
- `Core/Lib/Encoder.h`
- `Core/Lib/Led.cpp`
- `Core/Lib/Led.h`

### Step 2: Backup Current State (Optional)

Before rollback, you may want to backup the current state:

```bash
# Create a backup of current state
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
mkdir -p BACKUP/pre-rollback-${TIMESTAMP}
cp Core/Src/main.cpp BACKUP/pre-rollback-${TIMESTAMP}/
cp Core/Lib/Tasks.* BACKUP/pre-rollback-${TIMESTAMP}/
cp Core/Lib/Oled.* BACKUP/pre-rollback-${TIMESTAMP}/
```

### Step 3: Restore Files

Copy files from backup to active locations:

```bash
# Restore main.cpp
cp BACKUP/v0.3-oled/Core/Src/main.cpp Core/Src/main.cpp

# Restore Tasks files
cp BACKUP/v0.3-oled/Core/Lib/Tasks.cpp Core/Lib/Tasks.cpp
cp BACKUP/v0.3-oled/Core/Lib/Tasks.h Core/Lib/Tasks.h

# Restore OLED files
cp BACKUP/v0.3-oled/Core/Lib/Oled.cpp Core/Lib/Oled.cpp
cp BACKUP/v0.3-oled/Core/Lib/Oled.hpp Core/Lib/Oled.hpp

# Restore other library files (if needed)
cp BACKUP/v0.3-oled/Core/Lib/Encoder.cpp Core/Lib/Encoder.cpp
cp BACKUP/v0.3-oled/Core/Lib/Encoder.h Core/Lib/Encoder.h
cp BACKUP/v0.3-oled/Core/Lib/Led.cpp Core/Lib/Led.cpp
cp BACKUP/v0.3-oled/Core/Lib/Led.h Core/Lib/Led.h
```

### Step 4: Clean Build Directory

Remove old build artifacts:

```bash
cd build
rm -rf CMakeFiles/
rm -f encoder-only.elf encoder-only.map
rm -f CMakeCache.txt
rm -f .ninja_*
```

### Step 5: Rebuild Project

Reconfigure and build:

```bash
# From build directory
cmake ..
ninja

# Check build output for success
echo $?  # Should return 0 for success
```

### Step 6: Verify Restored Version

Check that the restored version matches expectations:

```bash
# Check memory usage (should match VERSION_INFO.md)
arm-none-eabi-size encoder-only.elf

# Expected output:
#    text    data     bss     dec     hex filename
#   59276       0   27072   86348   15140 encoder-only.elf
```

Verify key features in code:

```bash
# Check for OLED initialization in main.cpp
grep -n "new display::Oled" ../Core/Src/main.cpp

# Check for position display in Tasks.cpp
grep -n "Pos: %d" ../Core/Lib/Tasks.cpp

# Check for g_oled export in Tasks.h
grep -n "extern display::Oled" ../Core/Lib/Tasks.h
```

### Step 7: Flash and Test

Flash the firmware to device:

```bash
# Using OpenOCD (adjust interface/target as needed)
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg \
  -c "program encoder-only.elf verify reset exit"

# Or using st-flash
st-flash write encoder-only.bin 0x08000000

# Or using STM32CubeProgrammer CLI
STM32_Programmer_CLI -c port=SWD -w encoder-only.elf -v -rst
```

Test checklist:
- [ ] Device boots successfully
- [ ] LED blinks at 500ms interval
- [ ] OLED displays "Sys Started"
- [ ] OLED displays "Pos: 0" on startup
- [ ] Encoder position updates on OLED when rotated
- [ ] UART outputs encoder events at 115200 baud
- [ ] USB CDC outputs encoder events
- [ ] Button press logged to UART/USB
- [ ] Long press resets position to 0

## Troubleshooting Rollback Issues

### Issue: Build Fails After Restore

**Solution 1**: Verify all files were copied correctly
```bash
diff BACKUP/v0.3-oled/Core/Src/main.cpp Core/Src/main.cpp
```

**Solution 2**: Clean rebuild
```bash
cd build
rm -rf *
cmake ..
ninja
```

**Solution 3**: Check for missing dependencies
```bash
# Verify sh1106 driver files exist
ls -la Core/Src/sh1106*
```

### Issue: OLED Not Displaying After Restore

**Possible causes**:
1. I2C not properly initialized - check `MX_I2C1_Init()` in main.cpp
2. OLED object not created - check `oled = new display::Oled(&hi2c1)`
3. Display not updated - check `oled->update()` calls
4. Hardware connection issue - verify I2C wiring

**Debug steps**:
```bash
# Check if OLED initialization code exists
grep -A 5 "new display::Oled" Core/Src/main.cpp

# Check if g_oled is set in main.cpp
grep "g_oled = oled" Core/Src/main.cpp

# Check if testTask uses g_oled
grep -A 10 "g_oled" Core/Lib/Tasks.cpp
```

### Issue: Encoder Position Not Updating on Display

**Check**:
1. Verify `g_oled != nullptr` check in testTask
2. Verify position change detection: `pos != last_position`
3. Check display update logic in Tasks.cpp line 95-104

```bash
# Verify display update code
sed -n '95,104p' Core/Lib/Tasks.cpp
```

### Issue: Wrong Memory Usage After Build

If memory usage differs significantly from expected:

```
Expected:
RAM:   27072 B (41.31%)
FLASH: 59276 B (22.61%)
```

**Solution**: Check compiler optimization settings in CMakeLists.txt

## File Checksums (for verification)

To verify file integrity, you can use checksums:

```bash
# Generate checksums for backup files
cd BACKUP/v0.3-oled
find . -type f -name "*.cpp" -o -name "*.h" -o -name "*.hpp" | \
  xargs sha256sum > checksums.txt

# Later, verify integrity
sha256sum -c checksums.txt
```

## Additional Notes

### Important Considerations
- This rollback only restores source code, not hardware configuration
- IOC file changes (if any) are NOT included in this backup
- CMakeLists.txt is NOT backed up - ensure it's compatible
- Driver files (sh1106.c, sh1106_font.c) are assumed unchanged

### What's NOT Included in This Backup
- Build directory contents (except .elf and .map files)
- CMake configuration files
- .ioc project file
- Git history (if using version control)
- HAL/driver library files
- Middleware files

### Restoring Binary Only (Quick Test)

If you just want to test the firmware without rebuilding:

```bash
# Flash the backed-up binary directly
st-flash write BACKUP/v0.3-oled/encoder-only.elf 0x08000000

# Or using OpenOCD
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg \
  -c "program BACKUP/v0.3-oled/encoder-only.elf verify reset exit"
```

## Support

If you encounter issues during rollback:
1. Check VERSION_INFO.md for feature details
2. Compare your files with backup files using `diff`
3. Verify hardware connections match the configuration
4. Check build warnings/errors for hints
5. Ensure all dependencies (HAL, FreeRTOS, USB) are properly configured

## Script for Automated Rollback

Save this as `rollback-v0.3-oled.sh` in project root:

```bash
#!/bin/bash

# Automated rollback script for v0.3-oled
# Usage: ./rollback-v0.3-oled.sh

PROJECT_ROOT="/home/victor/STM/PROJECTS/VSCODE/encoder-only"
BACKUP_DIR="$PROJECT_ROOT/BACKUP/v0.3-oled"

echo "Starting rollback to v0.3-oled..."

# Check if backup exists
if [ ! -d "$BACKUP_DIR" ]; then
    echo "ERROR: Backup directory not found: $BACKUP_DIR"
    exit 1
fi

# Restore files
echo "Restoring source files..."
cp "$BACKUP_DIR/Core/Src/main.cpp" "$PROJECT_ROOT/Core/Src/"
cp "$BACKUP_DIR/Core/Lib/Tasks.cpp" "$PROJECT_ROOT/Core/Lib/"
cp "$BACKUP_DIR/Core/Lib/Tasks.h" "$PROJECT_ROOT/Core/Lib/"
cp "$BACKUP_DIR/Core/Lib/Oled.cpp" "$PROJECT_ROOT/Core/Lib/"
cp "$BACKUP_DIR/Core/Lib/Oled.hpp" "$PROJECT_ROOT/Core/Lib/"
cp "$BACKUP_DIR/Core/Lib/Encoder.cpp" "$PROJECT_ROOT/Core/Lib/"
cp "$BACKUP_DIR/Core/Lib/Encoder.h" "$PROJECT_ROOT/Core/Lib/"
cp "$BACKUP_DIR/Core/Lib/Led.cpp" "$PROJECT_ROOT/Core/Lib/"
cp "$BACKUP_DIR/Core/Lib/Led.h" "$PROJECT_ROOT/Core/Lib/"

echo "Files restored successfully"

# Rebuild
echo "Rebuilding project..."
cd "$PROJECT_ROOT/build"
ninja

if [ $? -eq 0 ]; then
    echo "Build successful!"
    echo "Rollback to v0.3-oled completed"
    arm-none-eabi-size encoder-only.elf
else
    echo "ERROR: Build failed"
    exit 1
fi
```

Make it executable:
```bash
chmod +x rollback-v0.3-oled.sh
```
