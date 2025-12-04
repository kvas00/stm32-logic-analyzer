#ifndef ENCODER_H
#define ENCODER_H

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

class Encoder {
public:
    Encoder(GPIO_TypeDef* portA, uint16_t pinA,
            GPIO_TypeDef* portB, uint16_t pinB,
            GPIO_TypeDef* portBtn, uint16_t pinBtn);

    void init();
    void update();
    void handleInterrupt();

    int getPosition() const { return position; }
    void resetPosition() { position = 0; }
    int getDelta() { int d = delta; delta = 0; return d; }

    bool isButtonPressed() const { return button_state == 0; }
    bool isButtonReleased() const { return button_state == 1; }
    bool isLongPress() const { return long_press_detected; }

private:
    // GPIO pins
    GPIO_TypeDef* port_a;
    uint16_t pin_a;
    GPIO_TypeDef* port_b;
    uint16_t pin_b;
    GPIO_TypeDef* port_button;
    uint16_t pin_button;

    // Encoder state
    int last_a_state;
    volatile int encoder_pos;
    volatile int delta;
    int position;
    bool direction;

    // Button state
    int button_state;
    int last_button_state;
    uint32_t last_debounce_time;
    uint32_t press_start_time;
    bool last_logged_button_state;
    bool long_press_detected;

    // Constants
    static const uint32_t DEBOUNCE_DELAY = 20;
    static const uint32_t LONG_PRESS_DURATION = 1000;
};

#endif // ENCODER_H
