/**
  ******************************************************************************
  * @file           : sh1106.h
  * @brief          : Header for SH1106 OLED display driver (128x64)
  * @author         : Based on U8g2 library initialization sequence
  ******************************************************************************
  * SH1106 is a 132x64 dot matrix OLED/PLED controller with 128x64 active area
  * I2C Address: 0x3C or 0x3D (0x78/0x7A in 8-bit format)
  *
  * Hardware connections (STM32F401CCUx):
  *   I2C1_SCL: PB6
  *   I2C1_SDA: PB7
  *   VCC: 3.3V
  *   GND: GND
  ******************************************************************************
  */

#ifndef SH1106_H
#define SH1106_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

/* ==================== Configuration ==================== */

#define SH1106_I2C_ADDR         (0x3C << 1)  // 0x78 in 8-bit format
#define SH1106_WIDTH            128
#define SH1106_HEIGHT           64
#define SH1106_PAGES            (SH1106_HEIGHT / 8)  // 8 pages
#define SH1106_COLUMN_OFFSET    2  // SH1106 uses 132x64, starts at column 2

/* ==================== Command/Data Control ==================== */

#define SH1106_CONTROL_BYTE_CMD_SINGLE    0x80
#define SH1106_CONTROL_BYTE_CMD_STREAM    0x00
#define SH1106_CONTROL_BYTE_DATA_STREAM   0x40

/* ==================== SH1106 Commands ==================== */

// Fundamental Commands
#define SH1106_CMD_SET_CONTRAST           0x81  // Followed by contrast value (0x00-0xFF)
#define SH1106_CMD_DISPLAY_ALL_ON_RESUME  0xA4  // Resume to RAM content display
#define SH1106_CMD_DISPLAY_ALL_ON         0xA5  // Entire display ON (ignore RAM)
#define SH1106_CMD_NORMAL_DISPLAY         0xA6  // Normal display (0=OFF, 1=ON)
#define SH1106_CMD_INVERSE_DISPLAY        0xA7  // Inverse display (0=ON, 1=OFF)
#define SH1106_CMD_DISPLAY_OFF            0xAE  // Display OFF (sleep mode)
#define SH1106_CMD_DISPLAY_ON             0xAF  // Display ON

// Addressing Setting Commands
#define SH1106_CMD_SET_PAGE_ADDR          0xB0  // Set page address (0xB0-0xB7)
#define SH1106_CMD_SET_COLUMN_ADDR_LOW    0x00  // Set lower column address (0x00-0x0F)
#define SH1106_CMD_SET_COLUMN_ADDR_HIGH   0x10  // Set higher column address (0x10-0x1F)

// Hardware Configuration Commands
#define SH1106_CMD_SET_START_LINE         0x40  // Set display start line (0x40-0x7F)
#define SH1106_CMD_SET_SEGMENT_REMAP_0    0xA0  // Column 0 mapped to SEG0
#define SH1106_CMD_SET_SEGMENT_REMAP_127  0xA1  // Column 127 mapped to SEG0
#define SH1106_CMD_SET_MULTIPLEX_RATIO    0xA8  // Followed by ratio (0x00-0x3F)
#define SH1106_CMD_SET_COM_SCAN_NORMAL    0xC0  // Normal COM output scan direction
#define SH1106_CMD_SET_COM_SCAN_REMAP     0xC8  // Remapped COM output scan direction
#define SH1106_CMD_SET_DISPLAY_OFFSET     0xD3  // Followed by offset (0x00-0x3F)
#define SH1106_CMD_SET_COM_PINS           0xDA  // Followed by config byte
#define SH1106_CMD_SET_DC_DC              0xAD  // Followed by DC-DC config (0x8A=OFF, 0x8B=ON)

// Timing & Driving Scheme Commands
#define SH1106_CMD_SET_DISPLAY_CLOCK_DIV  0xD5  // Followed by divide ratio/osc freq
#define SH1106_CMD_SET_PRECHARGE_PERIOD   0xD9  // Followed by pre-charge period
#define SH1106_CMD_SET_VCOM_DESELECT      0xDB  // Followed by VCOM deselect level
#define SH1106_CMD_SET_PUMP_VOLTAGE       0x30  // Pump voltage value (0x30-0x33)

// Read-Modify-Write Commands
#define SH1106_CMD_READ_MODIFY_WRITE      0xE0  // Enter read-modify-write mode
#define SH1106_CMD_END_READ_MODIFY_WRITE  0xEE  // Exit read-modify-write mode
#define SH1106_CMD_NOP                    0xE3  // No operation

/* ==================== Structure ==================== */

typedef struct {
    I2C_HandleTypeDef *hi2c;
    uint8_t address;
    uint8_t framebuffer[SH1106_WIDTH * SH1106_PAGES];      // 1024 bytes (128x64/8) - current buffer
    uint8_t prev_framebuffer[SH1106_WIDTH * SH1106_PAGES]; // 1024 bytes - previous buffer for delta detection
    bool initialized;
} SH1106_t;

/* ==================== Public Functions ==================== */

/**
 * @brief Initialize SH1106 display
 * @param dev Pointer to SH1106 device structure
 * @param hi2c Pointer to I2C handle
 * @retval HAL_OK if successful, HAL_ERROR otherwise
 */
HAL_StatusTypeDef SH1106_Init(SH1106_t *dev, I2C_HandleTypeDef *hi2c);

/**
 * @brief Turn display ON
 * @param dev Pointer to SH1106 device structure
 * @retval HAL_OK if successful, HAL_ERROR otherwise
 */
HAL_StatusTypeDef SH1106_DisplayOn(SH1106_t *dev);

/**
 * @brief Turn display OFF
 * @param dev Pointer to SH1106 device structure
 * @retval HAL_OK if successful, HAL_ERROR otherwise
 */
HAL_StatusTypeDef SH1106_DisplayOff(SH1106_t *dev);

/**
 * @brief Clear entire display (set all pixels to OFF)
 * @param dev Pointer to SH1106 device structure
 * @retval HAL_OK if successful, HAL_ERROR otherwise
 */
HAL_StatusTypeDef SH1106_Clear(SH1106_t *dev);

/**
 * @brief Fill entire display (set all pixels to ON)
 * @param dev Pointer to SH1106 device structure
 * @retval HAL_OK if successful, HAL_ERROR otherwise
 */
HAL_StatusTypeDef SH1106_Fill(SH1106_t *dev);

/**
 * @brief Update display with framebuffer content
 * @param dev Pointer to SH1106 device structure
 * @retval HAL_OK if successful, HAL_ERROR otherwise
 */
HAL_StatusTypeDef SH1106_UpdateScreen(SH1106_t *dev);

/**
 * @brief Set pixel in framebuffer
 * @param dev Pointer to SH1106 device structure
 * @param x X coordinate (0-127)
 * @param y Y coordinate (0-63)
 * @param color 1=ON, 0=OFF
 */
void SH1106_SetPixel(SH1106_t *dev, uint8_t x, uint8_t y, uint8_t color);

/**
 * @brief Draw character at specified position (5x7 font)
 * @param dev Pointer to SH1106 device structure
 * @param x X coordinate (column, 0-127)
 * @param y Y coordinate (row, 0-63)
 * @param ch Character to draw (ASCII 0x20-0x7E)
 * @param color 1=white, 0=black
 * @retval Width of character drawn (6 pixels including 1px spacing)
 */
uint8_t SH1106_DrawChar(SH1106_t *dev, uint8_t x, uint8_t y, char ch, uint8_t color);

/**
 * @brief Draw string at specified position (5x7 font)
 * @param dev Pointer to SH1106 device structure
 * @param x X coordinate (column, 0-127)
 * @param y Y coordinate (row, 0-63)
 * @param str String to draw (null-terminated)
 * @param color 1=white, 0=black
 * @retval Width of string drawn in pixels
 */
uint16_t SH1106_DrawString(SH1106_t *dev, uint8_t x, uint8_t y, const char *str, uint8_t color);

/**
 * @brief Set contrast (brightness)
 * @param dev Pointer to SH1106 device structure
 * @param contrast Contrast value (0x00-0xFF)
 * @retval HAL_OK if successful, HAL_ERROR otherwise
 */
HAL_StatusTypeDef SH1106_SetContrast(SH1106_t *dev, uint8_t contrast);

/**
 * @brief Invert display colors
 * @param dev Pointer to SH1106 device structure
 * @param invert true=inverted, false=normal
 * @retval HAL_OK if successful, HAL_ERROR otherwise
 */
HAL_StatusTypeDef SH1106_InvertDisplay(SH1106_t *dev, bool invert);

#ifdef __cplusplus
}
#endif

#endif /* SH1106_H */
