#ifndef LED_H
#define LED_H

#include "main.h"

class Led {
public:
    Led(GPIO_TypeDef* port, uint16_t pin, bool activeHigh = true);

    void on();
    void off();
    void toggle();
    void blink(uint32_t duration_ms);
    void doubleBlink();
    void singleBlink();

private:
    GPIO_TypeDef* gpio_port;
    uint16_t gpio_pin;
    bool active_high;
};

#endif // LED_H
