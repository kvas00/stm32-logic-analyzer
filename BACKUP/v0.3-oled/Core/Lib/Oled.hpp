/**
  ******************************************************************************
  * @file           : Oled.hpp
  * @brief          : C++ wrapper class for SH1106 OLED display
  ******************************************************************************
  * Object-oriented interface for SH1106 128x64 OLED display
  * Wraps low-level C driver functions in a clean C++ API
  ******************************************************************************
  */

#ifndef OLED_HPP
#define OLED_HPP

#include "stm32f4xx_hal.h"
#include "sh1106.h"
#include <cstdint>
#include <cstring>

namespace display {

/**
 * @brief OLED display class for SH1106 controller
 *
 * Provides object-oriented interface to SH1106 OLED display.
 * Supports text rendering, graphics, and display control.
 */
class Oled {
public:
    /**
     * @brief Construct a new Oled object
     * @param hi2c Pointer to I2C handle
     */
    explicit Oled(I2C_HandleTypeDef* hi2c);

    /**
     * @brief Destroy the Oled object
     */
    ~Oled() = default;

    // Prevent copying
    Oled(const Oled&) = delete;
    Oled& operator=(const Oled&) = delete;

    // Allow moving
    Oled(Oled&&) = default;
    Oled& operator=(Oled&&) = default;

    /**
     * @brief Initialize the display
     * @return true if successful, false otherwise
     */
    bool init();

    /**
     * @brief Turn display ON
     * @return true if successful, false otherwise
     */
    bool displayOn();

    /**
     * @brief Turn display OFF
     * @return true if successful, false otherwise
     */
    bool displayOff();

    /**
     * @brief Clear entire display (all pixels OFF)
     * @return true if successful, false otherwise
     */
    bool clear();

    /**
     * @brief Fill entire display (all pixels ON)
     * @return true if successful, false otherwise
     */
    bool fill();

    /**
     * @brief Update display with framebuffer content
     * @return true if successful, false otherwise
     */
    bool update();

    /**
     * @brief Set a single pixel
     * @param x X coordinate (0-127)
     * @param y Y coordinate (0-63)
     * @param color 1=white, 0=black
     */
    void setPixel(uint8_t x, uint8_t y, uint8_t color);

    /**
     * @brief Draw a character at specified position
     * @param x X coordinate
     * @param y Y coordinate
     * @param ch Character to draw (ASCII 0x20-0x7E)
     * @param color 1=white, 0=black
     * @return Width of character drawn (pixels)
     */
    uint8_t drawChar(uint8_t x, uint8_t y, char ch, uint8_t color = 1);

    /**
     * @brief Draw a string at specified position
     * @param x X coordinate
     * @param y Y coordinate
     * @param str String to draw (null-terminated)
     * @param color 1=white, 0=black
     * @return Width of string drawn (pixels)
     */
    uint16_t drawString(uint8_t x, uint8_t y, const char* str, uint8_t color = 1);

    /**
     * @brief Set display contrast (brightness)
     * @param contrast Contrast value (0x00-0xFF)
     * @return true if successful, false otherwise
     */
    bool setContrast(uint8_t contrast);

    /**
     * @brief Invert display colors
     * @param invert true=inverted, false=normal
     * @return true if successful, false otherwise
     */
    bool invertDisplay(bool invert);

    /**
     * @brief Check if display is initialized
     * @return true if initialized, false otherwise
     */
    bool isInitialized() const { return device_.initialized; }

    /**
     * @brief Get display width
     * @return Width in pixels (128)
     */
    static constexpr uint8_t getWidth() { return SH1106_WIDTH; }

    /**
     * @brief Get display height
     * @return Height in pixels (64)
     */
    static constexpr uint8_t getHeight() { return SH1106_HEIGHT; }

private:
    SH1106_t device_;  ///< Low-level device structure
};

} // namespace display

#endif /* OLED_HPP */
