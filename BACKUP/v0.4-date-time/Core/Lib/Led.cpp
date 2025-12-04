#include "Led.h"

Led::Led(GPIO_TypeDef* port, uint16_t pin, bool activeHigh)
    : gpio_port(port), gpio_pin(pin), active_high(activeHigh) {
}

void Led::on() {
    if (active_high) {
        HAL_GPIO_WritePin(gpio_port, gpio_pin, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(gpio_port, gpio_pin, GPIO_PIN_RESET);
    }
}

void Led::off() {
    if (active_high) {
        HAL_GPIO_WritePin(gpio_port, gpio_pin, GPIO_PIN_RESET);
    } else {
        HAL_GPIO_WritePin(gpio_port, gpio_pin, GPIO_PIN_SET);
    }
}

void Led::toggle() {
    HAL_GPIO_TogglePin(gpio_port, gpio_pin);
}

void Led::blink(uint32_t duration_ms) {
    on();
    HAL_Delay(duration_ms);
    off();
}

void Led::doubleBlink() {
    blink(50);
    HAL_Delay(50);
    blink(50);
}

void Led::singleBlink() {
    blink(100);
}
