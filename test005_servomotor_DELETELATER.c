
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define SERVO_PIN 21

// Set servo position (0 to 180 degrees)
void servo_move(uint angle) {
    if (angle > 180) angle = 180;

    // Pulse width: 1000 to 2000 microseconds
    uint16_t pulse_us = 1000 + (angle * 1000 / 180);

    // PWM clock: 1 MHz (1 count = 1 us)
    pwm_set_gpio_level(SERVO_PIN, pulse_us);
}

int main() {
    stdio_init_all();

    // Configure GPIO 21 for PWM
    gpio_set_function(SERVO_PIN, GPIO_FUNC_PWM);

    uint slice = pwm_gpio_to_slice_num(SERVO_PIN);

    // 125 MHz / 125 = 1 MHz
    pwm_set_clkdiv(slice, 125.0f);

    // 20 ms period = 50 Hz
    pwm_set_wrap(slice, 19999);

    // Start PWM
    pwm_set_enabled(slice, true);

    while (true) {

        // Move to 0 degrees
        servo_move(0);
        sleep_ms(1000);

        // Move to 180 degrees
        servo_move(180);
        sleep_ms(1000);
    }
}

