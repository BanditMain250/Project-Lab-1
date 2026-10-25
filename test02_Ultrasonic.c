
#include <stdio.h>
#include "pico/stdlib.h"

#define LED_PIN 0
#define TRIG_PIN 16
#define ECHO_PIN 17

#define DETECT_DISTANCE_CM 7.62f

float get_distance_cm(void)
{
    // Send 10 microsecond pulse to TRIG
    gpio_put(TRIG_PIN, 0);
    sleep_us(2);

    gpio_put(TRIG_PIN, 1);
    sleep_us(10);
    gpio_put(TRIG_PIN, 0);

    // Wait for ECHO to go HIGH (with timeout)
    absolute_time_t timeout = make_timeout_time_ms(30);

    while (gpio_get(ECHO_PIN) == 0) {
        if (time_reached(timeout)) {
            return -1.0f;
        }
    }

    // Start measuring ECHO pulse duration
    uint64_t start = time_us_64();

    timeout = make_timeout_time_ms(30);

    while (gpio_get(ECHO_PIN) == 1) {
        if (time_reached(timeout)) {
            return -1.0f;
        }
    }

    uint64_t end = time_us_64();

    // Convert round-trip time into distance (cm)
    float duration = (float)(end - start);
    float distance = (duration * 0.0343f) / 2.0f;

    return distance;
}

int main()
{
    stdio_init_all();

    // Initialize LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 0);

    // Initialize TRIG
    gpio_init(TRIG_PIN);
    gpio_set_dir(TRIG_PIN, GPIO_OUT);
    gpio_put(TRIG_PIN, 0);

    // Initialize ECHO
    gpio_init(ECHO_PIN);
    gpio_set_dir(ECHO_PIN, GPIO_IN);

    while (true)
    {
        float distance = get_distance_cm();

        if (distance > 0 && distance <= DETECT_DISTANCE_CM)
        {
            gpio_put(LED_PIN, 1);  // LED ON
        }
        else
        {
            gpio_put(LED_PIN, 0);  // LED OFF
        }

        printf("Distance: %.2f cm\n", distance);

        sleep_ms(60);
    }
}
