/**
  ******************************************************************************
  * @file           : Oled.cpp
  * @brief          : C++ wrapper class implementation for SH1106 OLED display
  ******************************************************************************
  */

#include "Oled.hpp"

namespace display {

Oled::Oled(I2C_HandleTypeDef* hi2c) {
    device_.hi2c = hi2c;
    device_.address = SH1106_I2C_ADDR;
    device_.initialized = false;
    std::memset(device_.framebuffer, 0, sizeof(device_.framebuffer));
    std::memset(device_.prev_framebuffer, 0, sizeof(device_.prev_framebuffer));
}

bool Oled::init() {
    return SH1106_Init(&device_, device_.hi2c) == HAL_OK;
}

bool Oled::displayOn() {
    return SH1106_DisplayOn(&device_) == HAL_OK;
}

bool Oled::displayOff() {
    return SH1106_DisplayOff(&device_) == HAL_OK;
}

bool Oled::clear() {
    return SH1106_Clear(&device_) == HAL_OK;
}

bool Oled::fill() {
    return SH1106_Fill(&device_) == HAL_OK;
}

bool Oled::update() {
    return SH1106_UpdateScreen(&device_) == HAL_OK;
}

void Oled::setPixel(uint8_t x, uint8_t y, uint8_t color) {
    SH1106_SetPixel(&device_, x, y, color);
}

uint8_t Oled::drawChar(uint8_t x, uint8_t y, char ch, uint8_t color) {
    return SH1106_DrawChar(&device_, x, y, ch, color);
}

uint16_t Oled::drawString(uint8_t x, uint8_t y, const char* str, uint8_t color) {
    return SH1106_DrawString(&device_, x, y, str, color);
}

bool Oled::setContrast(uint8_t contrast) {
    return SH1106_SetContrast(&device_, contrast) == HAL_OK;
}

bool Oled::invertDisplay(bool invert) {
    return SH1106_InvertDisplay(&device_, invert) == HAL_OK;
}

void Oled::drawDottedLine50(uint8_t x, uint8_t y, uint8_t width,
                           uint8_t spacing, uint8_t color) {
    // Draw horizontal dotted line with 50% brightness using checkerboard pattern
    // Every other dot is drawn to create 50% brightness effect
    for (uint8_t px = x; px < (x + width) && px < SH1106_WIDTH; px += spacing) {
        // Checkerboard pattern: draw pixel only if (px/spacing) is even
        if (((px - x) / spacing) % 2 == 0) {
            if (y < SH1106_HEIGHT) {
                setPixel(px, y, color);
            }
        }
    }
}

void Oled::drawLogicSignal(uint8_t x, uint8_t y, const uint8_t* signal_data,
                          uint16_t data_length, uint8_t height, uint16_t x_offset,
                          float zoom_factor, uint8_t color) {
    if (signal_data == nullptr || data_length == 0) {
        return;
    }

    float current_x_float = (float)x - (float)x_offset;  // Apply offset (can be negative)
    uint8_t current_value = 0;  // Start with LOW (0)

    for (uint16_t i = 0; i < data_length; i++) {
        uint8_t data_byte = signal_data[i];
        uint8_t value = (data_byte & 0x80) >> 7;  // Extract bit 7
        uint8_t delta = data_byte & 0x7F;         // Extract bits 6-0

        // Apply zoom to delta (time scaling)
        float zoomed_delta = (float)delta * zoom_factor;
        int16_t current_x = (int16_t)current_x_float;

        // Draw vertical transition if value changed
        if (i > 0 && value != current_value) {
            // Only draw if within screen bounds
            if (current_x >= 0 && current_x < SH1106_WIDTH) {
                uint8_t y_start = (current_value == 0) ? y + height - 1 : y;
                uint8_t y_end = (value == 0) ? y + height - 1 : y;

                // Draw vertical line
                for (uint8_t py = (y_start < y_end ? y_start : y_end);
                     py <= (y_start > y_end ? y_start : y_end); py++) {
                    if (py < SH1106_HEIGHT) {
                        setPixel(current_x, py, color);
                    }
                }
            }
        }

        current_value = value;

        // Draw horizontal line with zoomed delta
        uint8_t y_level = (current_value == 0) ? y + height - 1 : y;
        int16_t zoomed_delta_int = (int16_t)zoomed_delta;
        for (int16_t dx = 0; dx < zoomed_delta_int; dx++) {
            int16_t draw_x = current_x + dx;
            // Only draw if within screen bounds
            if (draw_x >= 0 && draw_x < SH1106_WIDTH && y_level < SH1106_HEIGHT) {
                setPixel(draw_x, y_level, color);
            }
        }

        current_x_float += zoomed_delta;

        // Stop if we've gone past the right edge of the display
        if (current_x_float >= SH1106_WIDTH) {
            break;
        }
    }
}

void Oled::drawLogicChannels(const uint8_t** channel_data, const uint16_t* data_lengths,
                            uint8_t num_channels, uint8_t start_y,
                            uint8_t channel_height, uint16_t x_offset, float zoom_factor,
                            uint8_t color) {
    if (channel_data == nullptr || data_lengths == nullptr || num_channels == 0) {
        return;
    }

    // Limit to 4 channels max
    if (num_channels > 4) {
        num_channels = 4;
    }

    uint8_t grid_start_x = 8;  // Start grid after label
    uint8_t grid_width = SH1106_WIDTH - grid_start_x;

    // Draw each channel
    for (uint8_t ch = 0; ch < num_channels; ch++) {
        uint8_t y_pos = start_y + (ch * channel_height);

        // Draw channel label (just the number: 0, 1, 2, 3)
        char label[2];
        label[0] = '0' + ch;
        label[1] = '\0';
        drawString(0, y_pos + 4, label, color);

        // Calculate baseline position (1 pixel above LOW level)
        // LOW level is at: y_pos + channel_height - 3
        // So baseline is at: y_pos + channel_height - 4
        uint8_t baseline_y = y_pos + channel_height - 4;

        // Draw dotted baseline with 50% brightness (1 pixel above LOW level)
        drawDottedLine50(grid_start_x, baseline_y, grid_width, 4, color);

        // Draw signal waveform (starting after label) with horizontal offset and zoom
        if (channel_data[ch] != nullptr && data_lengths[ch] > 0) {
            drawLogicSignal(grid_start_x, y_pos + 2, channel_data[ch], data_lengths[ch],
                           channel_height - 4, x_offset, zoom_factor, color);
        }
    }
}

} // namespace display
