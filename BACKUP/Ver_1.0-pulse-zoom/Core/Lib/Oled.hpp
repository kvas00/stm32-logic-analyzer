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

    /**
     * @brief Draw horizontal dotted line with 50% brightness (checkerboard pattern)
     * @param x X coordinate (starting position)
     * @param y Y coordinate
     * @param width Width of line
     * @param spacing Spacing between dots (in pixels)
     * @param color 1=white, 0=black
     */
    void drawDottedLine50(uint8_t x, uint8_t y, uint8_t width,
                         uint8_t spacing = 4, uint8_t color = 1);

    /**
     * @brief Draw logic analyzer signal
     * @param x X coordinate (starting position)
     * @param y Y coordinate (top of signal waveform)
     * @param signal_data Array of bytes representing signal data
     *                    Bit 7: signal value (0 or 1)
     *                    Bits 6-0: time delta from previous transition (in pixels)
     * @param data_length Number of bytes in signal_data array
     * @param height Height of the signal waveform in pixels
     * @param x_offset Horizontal scroll offset in pixels
     * @param zoom_factor Time scale factor (0.5x = compress, 2.0x = expand)
     * @param color 1=white, 0=black
     */
    void drawLogicSignal(uint8_t x, uint8_t y, const uint8_t* signal_data,
                        uint16_t data_length, uint8_t height, uint16_t x_offset = 0,
                        float zoom_factor = 1.0f, uint8_t color = 1);

    /**
     * @brief Draw multiple logic analyzer channels
     * @param channel_data Array of pointers to signal data for each channel
     * @param data_lengths Array of data lengths for each channel
     * @param num_channels Number of channels (max 4)
     * @param start_y Starting Y coordinate for first channel
     * @param channel_height Height of each channel in pixels
     * @param x_offset Horizontal scroll offset in pixels
     * @param zoom_factor Time scale factor (0.5x = compress, 2.0x = expand)
     * @param color 1=white, 0=black
     */
    void drawLogicChannels(const uint8_t** channel_data, const uint16_t* data_lengths,
                          uint8_t num_channels, uint8_t start_y = 0,
                          uint8_t channel_height = 16, uint16_t x_offset = 0,
                          float zoom_factor = 1.0f, uint8_t color = 1);

private:
    SH1106_t device_;  ///< Low-level device structure
};

} // namespace display

#endif /* OLED_HPP */
