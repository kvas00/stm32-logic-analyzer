/**
  ******************************************************************************
  * @file           : sh1106.c
  * @brief          : SH1106 OLED display driver implementation (128x64)
  * @author         : Based on U8g2 library initialization sequence
  ******************************************************************************
  * Initialization sequence based on U8g2 Winstar variant:
  * https://github.com/olikraus/u8g2/blob/master/csrc/u8x8_d_ssd1306_128x64_noname.c
  ******************************************************************************
  */

#include "sh1106.h"
#include "sh1106_font.h"
#include <string.h>
#include <stdio.h>

/* ==================== Private Defines ==================== */

#define SH1106_I2C_TIMEOUT  500  // I2C timeout in milliseconds (increased for data transfers)

/* ==================== Private Functions ==================== */

/**
 * @brief Write command byte to SH1106
 * @param dev Pointer to SH1106 device structure
 * @param cmd Command byte
 * @retval HAL_OK if successful, HAL_ERROR otherwise
 */
static HAL_StatusTypeDef SH1106_WriteCommand(SH1106_t *dev, uint8_t cmd)
{
    uint8_t data[2] = {SH1106_CONTROL_BYTE_CMD_SINGLE, cmd};
    return HAL_I2C_Master_Transmit(dev->hi2c, dev->address, data, 2, SH1106_I2C_TIMEOUT);
}

/**
 * @brief Write multiple command bytes to SH1106
 * @param dev Pointer to SH1106 device structure
 * @param cmds Array of command bytes
 * @param len Number of commands
 * @retval HAL_OK if successful, HAL_ERROR otherwise
 */
static HAL_StatusTypeDef SH1106_WriteCommands(SH1106_t *dev, const uint8_t *cmds, uint8_t len)
{
    HAL_StatusTypeDef status;
    for (uint8_t i = 0; i < len; i++) {
        status = SH1106_WriteCommand(dev, cmds[i]);
        if (status != HAL_OK) {
            return status;
        }
    }
    return HAL_OK;
}

/**
 * @brief Write data bytes to SH1106
 * @param dev Pointer to SH1106 device structure
 * @param data Data buffer
 * @param len Data length
 * @retval HAL_OK if successful, HAL_ERROR otherwise
 */
static HAL_StatusTypeDef SH1106_WriteData(SH1106_t *dev, const uint8_t *data, uint16_t len)
{
    // For larger transfers, we need to send control byte + data together
    // to avoid I2C STOP condition between them

    // Allocate buffer for control byte + data (on stack for small chunks)
    if (len <= 128) {
        uint8_t buffer[129];  // 1 control byte + up to 128 data bytes
        buffer[0] = SH1106_CONTROL_BYTE_DATA_STREAM;
        memcpy(&buffer[1], data, len);
        return HAL_I2C_Master_Transmit(dev->hi2c, dev->address, buffer, len + 1, SH1106_I2C_TIMEOUT);
    } else {
        // For larger transfers, send in chunks of 128 bytes
        HAL_StatusTypeDef status;
        uint16_t offset = 0;
        uint8_t buffer[129];

        while (offset < len) {
            uint16_t chunk_size = (len - offset > 128) ? 128 : (len - offset);
            buffer[0] = SH1106_CONTROL_BYTE_DATA_STREAM;
            memcpy(&buffer[1], data + offset, chunk_size);

            status = HAL_I2C_Master_Transmit(dev->hi2c, dev->address, buffer, chunk_size + 1, SH1106_I2C_TIMEOUT);
            if (status != HAL_OK) {
                return status;
            }

            offset += chunk_size;
        }

        return HAL_OK;
    }
}

/**
 * @brief Check if device is present on I2C bus at address 0x3C
 * @param hi2c Pointer to I2C handle
 * @retval true if device found, false otherwise
 */
static bool SH1106_CheckDevice(I2C_HandleTypeDef *hi2c)
{
    HAL_StatusTypeDef status;
    status = HAL_I2C_IsDeviceReady(hi2c, SH1106_I2C_ADDR, 3, SH1106_I2C_TIMEOUT);
    return (status == HAL_OK);
}

/* ==================== Public Functions ==================== */

HAL_StatusTypeDef SH1106_Init(SH1106_t *dev, I2C_HandleTypeDef *hi2c)
{
    HAL_StatusTypeDef status;

    // Initialize device structure
    dev->hi2c = hi2c;
    dev->address = SH1106_I2C_ADDR;
    dev->initialized = false;
    memset(dev->framebuffer, 0, sizeof(dev->framebuffer));

    // Check if device is present
    if (!SH1106_CheckDevice(hi2c)) {
        printf("SH1106: Device not found at address 0x%02X\r\n", SH1106_I2C_ADDR >> 1);
        return HAL_ERROR;
    }

    printf("SH1106: Device found at address 0x%02X\r\n", SH1106_I2C_ADDR >> 1);

    // Small delay after power-up
    HAL_Delay(10);

    /**
     * Initialization sequence based on U8g2 Winstar variant:
     * u8x8_d_sh1106_128x64_winstar_init_seq
     */

    // 1. Display OFF
    status = SH1106_WriteCommand(dev, SH1106_CMD_DISPLAY_OFF);
    if (status != HAL_OK) return status;

    // 2. Set Entire Display OFF (resume to RAM content)
    status = SH1106_WriteCommand(dev, SH1106_CMD_DISPLAY_ALL_ON_RESUME);
    if (status != HAL_OK) return status;

    // 3. Set Display Clock Divide Ratio/Oscillator Frequency
    // 0xD5, 0x50 (divide ratio = 1, frequency = +/- 0%)
    uint8_t clock_cmds[] = {SH1106_CMD_SET_DISPLAY_CLOCK_DIV, 0x50};
    status = SH1106_WriteCommands(dev, clock_cmds, 2);
    if (status != HAL_OK) return status;

    // 4. Set Multiplex Ratio (64 lines)
    // 0xA8, 0x3F
    uint8_t multiplex_cmds[] = {SH1106_CMD_SET_MULTIPLEX_RATIO, 0x3F};
    status = SH1106_WriteCommands(dev, multiplex_cmds, 2);
    if (status != HAL_OK) return status;

    // 5. Set Display Offset (0)
    // 0xD3, 0x00
    uint8_t offset_cmds[] = {SH1106_CMD_SET_DISPLAY_OFFSET, 0x00};
    status = SH1106_WriteCommands(dev, offset_cmds, 2);
    if (status != HAL_OK) return status;

    // 6. Set Display Start Line (0)
    // 0x40
    status = SH1106_WriteCommand(dev, SH1106_CMD_SET_START_LINE | 0x00);
    if (status != HAL_OK) return status;

    // 7. Set DC-DC Mode (Built-in DC-DC ON)
    // 0xAD, 0x8B
    uint8_t dcdc_cmds[] = {SH1106_CMD_SET_DC_DC, 0x8B};
    status = SH1106_WriteCommands(dev, dcdc_cmds, 2);
    if (status != HAL_OK) return status;

    // 8. Set Pre-charge/Discharge Period (2 DCLKs each)
    // 0xD9, 0x22
    uint8_t precharge_cmds[] = {SH1106_CMD_SET_PRECHARGE_PERIOD, 0x22};
    status = SH1106_WriteCommands(dev, precharge_cmds, 2);
    if (status != HAL_OK) return status;

    // 9. Set VCOM Deselect Level (0.770V)
    // 0xDB, 0x35
    uint8_t vcom_cmds[] = {SH1106_CMD_SET_VCOM_DESELECT, 0x35};
    status = SH1106_WriteCommands(dev, vcom_cmds, 2);
    if (status != HAL_OK) return status;

    // 10. Set Pump Voltage (8.0V)
    // 0x32
    status = SH1106_WriteCommand(dev, SH1106_CMD_SET_PUMP_VOLTAGE | 0x02);
    if (status != HAL_OK) return status;

    // 11. Set Contrast (maximum)
    // 0x81, 0xFF
    uint8_t contrast_cmds[] = {SH1106_CMD_SET_CONTRAST, 0xFF};
    status = SH1106_WriteCommands(dev, contrast_cmds, 2);
    if (status != HAL_OK) return status;

    // 12. Set Normal Display Mode (not inverted)
    // 0xA6
    status = SH1106_WriteCommand(dev, SH1106_CMD_NORMAL_DISPLAY);
    if (status != HAL_OK) return status;

    // 13. Set COM Pin Hardware Configuration
    // 0xDA, 0x12 (alternative COM pin config, disable COM left/right remap)
    uint8_t com_cmds[] = {SH1106_CMD_SET_COM_PINS, 0x12};
    status = SH1106_WriteCommands(dev, com_cmds, 2);
    if (status != HAL_OK) return status;

    // Additional recommended settings for proper orientation
    // Segment remap (column 127 mapped to SEG0)
    status = SH1106_WriteCommand(dev, SH1106_CMD_SET_SEGMENT_REMAP_127);
    if (status != HAL_OK) return status;

    // COM output scan direction (remapped)
    status = SH1106_WriteCommand(dev, SH1106_CMD_SET_COM_SCAN_REMAP);
    if (status != HAL_OK) return status;

    // Clear display
    status = SH1106_Clear(dev);
    if (status != HAL_OK) return status;

    // Display ON
    status = SH1106_WriteCommand(dev, SH1106_CMD_DISPLAY_ON);
    if (status != HAL_OK) return status;

    dev->initialized = true;
    printf("SH1106: Initialization complete\r\n");

    return HAL_OK;
}

HAL_StatusTypeDef SH1106_DisplayOn(SH1106_t *dev)
{
    return SH1106_WriteCommand(dev, SH1106_CMD_DISPLAY_ON);
}

HAL_StatusTypeDef SH1106_DisplayOff(SH1106_t *dev)
{
    return SH1106_WriteCommand(dev, SH1106_CMD_DISPLAY_OFF);
}

HAL_StatusTypeDef SH1106_Clear(SH1106_t *dev)
{
    // Clear framebuffer
    memset(dev->framebuffer, 0, sizeof(dev->framebuffer));

    // Update display
    return SH1106_UpdateScreen(dev);
}

HAL_StatusTypeDef SH1106_Fill(SH1106_t *dev)
{
    // Fill framebuffer
    memset(dev->framebuffer, 0xFF, sizeof(dev->framebuffer));

    // Update display
    return SH1106_UpdateScreen(dev);
}

HAL_StatusTypeDef SH1106_UpdateScreen(SH1106_t *dev)
{
    HAL_StatusTypeDef status;

    // SH1106 has 8 pages (rows of 8 pixels each)
    for (uint8_t page = 0; page < SH1106_PAGES; page++) {
        // Set page address (0xB0 - 0xB7)
        status = SH1106_WriteCommand(dev, SH1106_CMD_SET_PAGE_ADDR | page);
        if (status != HAL_OK) {
            printf("SH1106: Error setting page %d (status=%d)\r\n", page, status);
            return status;
        }

        // Set column address (with 2-pixel offset for SH1106)
        // Lower nibble
        status = SH1106_WriteCommand(dev, SH1106_CMD_SET_COLUMN_ADDR_LOW | (SH1106_COLUMN_OFFSET & 0x0F));
        if (status != HAL_OK) {
            printf("SH1106: Error setting column low (page=%d, status=%d)\r\n", page, status);
            return status;
        }

        // Higher nibble
        status = SH1106_WriteCommand(dev, SH1106_CMD_SET_COLUMN_ADDR_HIGH | ((SH1106_COLUMN_OFFSET >> 4) & 0x0F));
        if (status != HAL_OK) {
            printf("SH1106: Error setting column high (page=%d, status=%d)\r\n", page, status);
            return status;
        }

        // Write page data (128 bytes)
        status = SH1106_WriteData(dev, &dev->framebuffer[page * SH1106_WIDTH], SH1106_WIDTH);
        if (status != HAL_OK) {
            printf("SH1106: Error writing data (page=%d, status=%d)\r\n", page, status);
            return status;
        }
    }

    return HAL_OK;
}

void SH1106_SetPixel(SH1106_t *dev, uint8_t x, uint8_t y, uint8_t color)
{
    if (x >= SH1106_WIDTH || y >= SH1106_HEIGHT) {
        return;  // Out of bounds
    }

    uint8_t page = y / 8;
    uint8_t bit = y % 8;
    uint16_t index = page * SH1106_WIDTH + x;

    if (color) {
        dev->framebuffer[index] |= (1 << bit);   // Set pixel
    } else {
        dev->framebuffer[index] &= ~(1 << bit);  // Clear pixel
    }
}

uint8_t SH1106_DrawChar(SH1106_t *dev, uint8_t x, uint8_t y, char ch, uint8_t color)
{
    // Check if character is in supported range
    if (ch < FONT_FIRST_CHAR || ch > FONT_LAST_CHAR) {
        ch = '?';  // Replace unsupported chars with '?'
    }

    // Check bounds
    if (x + FONT_WIDTH > SH1106_WIDTH || y + FONT_HEIGHT > SH1106_HEIGHT) {
        return 0;  // Character would be out of bounds
    }

    // Get font data for this character
    uint8_t char_index = ch - FONT_FIRST_CHAR;
    const uint8_t *char_data = font5x7[char_index];

    // Draw character column by column
    for (uint8_t col = 0; col < FONT_WIDTH; col++) {
        uint8_t column_data = char_data[col];

        // Draw each pixel in the column
        for (uint8_t row = 0; row < FONT_HEIGHT; row++) {
            if (column_data & (1 << row)) {
                // Pixel is set in font data
                SH1106_SetPixel(dev, x + col, y + row, color);
            } else {
                // Pixel is clear in font data
                SH1106_SetPixel(dev, x + col, y + row, !color);
            }
        }
    }

    // Add 1 pixel spacing after character
    for (uint8_t row = 0; row < FONT_HEIGHT; row++) {
        SH1106_SetPixel(dev, x + FONT_WIDTH, y + row, !color);
    }

    return FONT_WIDTH + 1;  // Return width including spacing
}

uint16_t SH1106_DrawString(SH1106_t *dev, uint8_t x, uint8_t y, const char *str, uint8_t color)
{
    uint16_t total_width = 0;
    uint8_t current_x = x;

    while (*str) {
        // Check if we have room for another character
        if (current_x + FONT_WIDTH > SH1106_WIDTH) {
            break;  // String would overflow screen width
        }

        uint8_t char_width = SH1106_DrawChar(dev, current_x, y, *str, color);
        current_x += char_width;
        total_width += char_width;
        str++;
    }

    return total_width;
}

HAL_StatusTypeDef SH1106_SetContrast(SH1106_t *dev, uint8_t contrast)
{
    uint8_t cmds[] = {SH1106_CMD_SET_CONTRAST, contrast};
    return SH1106_WriteCommands(dev, cmds, 2);
}

HAL_StatusTypeDef SH1106_InvertDisplay(SH1106_t *dev, bool invert)
{
    uint8_t cmd = invert ? SH1106_CMD_INVERSE_DISPLAY : SH1106_CMD_NORMAL_DISPLAY;
    return SH1106_WriteCommand(dev, cmd);
}
