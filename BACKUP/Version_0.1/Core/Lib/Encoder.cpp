#include "Encoder.h"

Encoder::Encoder(GPIO_TypeDef* portA, uint16_t pinA,
                 GPIO_TypeDef* portB, uint16_t pinB,
                 GPIO_TypeDef* portBtn, uint16_t pinBtn)
    : port_a(portA), pin_a(pinA),
      port_b(portB), pin_b(pinB),
      port_button(portBtn), pin_button(pinBtn),
      last_a_state(0), encoder_pos(0), delta(0), position(0), direction(false),
      button_state(1), last_button_state(1),
      last_debounce_time(0), press_start_time(0),
      last_logged_button_state(1), long_press_detected(false) {
}

void Encoder::init() {
    last_a_state = HAL_GPIO_ReadPin(port_a, pin_a);
}

void Encoder::handleInterrupt() {
    int current_a = HAL_GPIO_ReadPin(port_a, pin_a);
    if (current_a != last_a_state) {
        if (HAL_GPIO_ReadPin(port_b, pin_b) != current_a) {
            encoder_pos++;
            direction = true;
            delta = 1;
        } else {
            encoder_pos--;
            direction = false;
            delta = -1;
        }
    }
    last_a_state = current_a;
}

void Encoder::update() {
    // Button handling
    int button_reading = HAL_GPIO_ReadPin(port_button, pin_button);
    if (button_reading != last_button_state) {
        last_debounce_time = HAL_GetTick();
        if (button_reading == 0) { // LOW - pressed
            press_start_time = HAL_GetTick();
        }
    }

    if ((HAL_GetTick() - last_debounce_time) > DEBOUNCE_DELAY) {
        if (button_reading != button_state) {
            button_state = button_reading;
            if (button_state != last_logged_button_state) {
                last_logged_button_state = button_state;
                long_press_detected = false;
            }
        }

        // Long press detection
        if (button_state == 0 && !long_press_detected &&
            (HAL_GetTick() - press_start_time) >= LONG_PRESS_DURATION) {
            long_press_detected = true;
            position = 0; // Reset position on long press
        }
    }
    last_button_state = button_reading;

    // Encoder rotation handling
    if (encoder_pos != 0) {
        int delta_val = encoder_pos;
        encoder_pos = 0;
        position += delta_val;
        delta = delta_val;
    }
}
