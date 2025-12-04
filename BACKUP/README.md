# Project Backups

This directory contains version backups of the encoder-only project.

## Available Backups

### v0.3-oled (2025-10-10) ✅ STABLE

**Features:**
- OLED display integration with SH1106 controller
- Real-time encoder position display
- Startup message: "Sys Started"
- Dynamic position updates: "Pos: X"
- Memory: RAM 41.31%, FLASH 22.61%

**Quick Restore:**
```bash
cd /home/victor/STM/PROJECTS/VSCODE/encoder-only
cp BACKUP/v0.3-oled/Core/Src/main.cpp Core/Src/
cp BACKUP/v0.3-oled/Core/Lib/Tasks.* Core/Lib/
cp BACKUP/v0.3-oled/Core/Lib/Oled.* Core/Lib/
cd build && ninja
```

**Documentation:**
- [README.md](v0.3-oled/README.md) - Quick start
- [VERSION_INFO.md](v0.3-oled/VERSION_INFO.md) - Detailed info
- [ROLLBACK_INSTRUCTIONS.md](v0.3-oled/ROLLBACK_INSTRUCTIONS.md) - Restore guide
- [MANIFEST.txt](v0.3-oled/MANIFEST.txt) - Complete file list

---

## Backup Structure

Each version backup contains:
- **Source files** - All modified C/C++ files
- **Binaries** - Compiled .elf and .map files
- **Documentation** - README, version info, rollback instructions
- **Checksums** - SHA256 checksums for verification
- **Manifest** - Complete file listing

## How to Use

1. **Browse available versions** in this directory
2. **Read the version README** to understand features
3. **Follow rollback instructions** to restore
4. **Verify checksums** after restore (optional)
5. **Rebuild and flash** to device

## Creating New Backups

When creating a new backup version:

```bash
# 1. Create version directory
mkdir -p BACKUP/vX.Y.Z/{Core/Src,Core/Lib,Core/Inc}

# 2. Copy modified files
cp Core/Src/modified_files.cpp BACKUP/vX.Y.Z/Core/Src/
cp Core/Lib/modified_files.* BACKUP/vX.Y.Z/Core/Lib/

# 3. Copy binaries
cp build/encoder-only.elf BACKUP/vX.Y.Z/
cp build/encoder-only.map BACKUP/vX.Y.Z/

# 4. Create documentation
# - README.md (quick start)
# - VERSION_INFO.md (detailed info)
# - ROLLBACK_INSTRUCTIONS.md (restore guide)
# - MANIFEST.txt (file list)

# 5. Generate checksums
cd BACKUP/vX.Y.Z
sha256sum Core/**/*.cpp Core/**/*.h *.elf > checksums.sha256

# 6. Update project CHANGELOG.md
```

## Version History

- **v0.3-oled** (2025-10-10) - OLED display with position monitoring [Current]
- **v0.2** - FreeRTOS integration with encoder
- **v0.1** - Initial baseline

## Notes

- Backups are **read-only** - don't modify files directly
- Always **test after restore** to ensure functionality
- **Verify checksums** if file integrity is critical
- Keep backups **clean and documented**
- Only backup **tested and stable** versions

## Related Files

- [../CHANGELOG.md](../CHANGELOG.md) - Project changelog
- [../FREERTOS_TASKS.md](../FREERTOS_TASKS.md) - FreeRTOS documentation
- [../CODE_STYLE.md](../CODE_STYLE.md) - Coding standards

---

For questions or issues, refer to the specific version's documentation.
