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

} // namespace display
