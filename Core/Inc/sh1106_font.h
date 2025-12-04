/**
  ******************************************************************************
  * @file           : sh1106_font.h
  * @brief          : Font definition for SH1106 OLED display
  ******************************************************************************
  * 5x7 monospace font for ASCII characters (0x20-0x7F)
  * Each character is 5 pixels wide, 7 pixels tall
  * Stored as 7 bytes per character (columns)
  ******************************************************************************
  */

#ifndef SH1106_FONT_H
#define SH1106_FONT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define FONT_WIDTH  5
#define FONT_HEIGHT 7
#define FONT_FIRST_CHAR 0x20  // Space
#define FONT_LAST_CHAR  0x7E  // ~

/**
 * @brief 5x7 font data
 * Each character is 5 bytes (columns), each byte is one column (7 bits used)
 * Characters from 0x20 (space) to 0x7E (~)
 */
extern const uint8_t font5x7[][5];

#ifdef __cplusplus
}
#endif

#endif /* SH1106_FONT_H */
