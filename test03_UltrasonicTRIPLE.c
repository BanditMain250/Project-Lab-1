
#include <stdio.h>
#include "pico/stdlib.h"

// LED pins
#define LED1 0
#define LED2 1
#define LED3 2

// Ultrasonic Sensor 1
#define TRIG1 16
#define ECHO1 17

// Ultrasonic Sensor 2
#define TRIG2 20
#define ECHO2 21

// Ultrasonic Sensor 3
#define TRIG3 26
#define ECHO3 27

#define DETECT_DISTANCE_CM 7.62f

// Initialize an ultrasonic sensor
void init_ultrasonic(uint trig_pin, uint echo_pin)
{
    gpio_init(trig_pin);
    gpio_set_dir(trig_pin, GPIO_OUT);
    gpio_put(trig_pin, 0);

    gpio_init(echo_pin);
    gpio_set_dir(echo_pin, GPIO_IN);
}

// Initialize an LED
void init_led(uint led_pin)
{
    gpio_init(led_pin);
    gpio_set_dir(led_pin, GPIO_OUT);
    gpio_put(led_pin, 0);
}

// Measure distance in centimeters
float get_distance_cm(uint trig_pin, uint echo_pin)
{
    // Send 10 microsecond trigger pulse
    gpio_put(trig_pin, 0);
    sleep_us(2);

    gpio_put(trig_pin, 1);
    sleep_us(10);
    gpio_put(trig_pin, 0);

    // Wait for ECHO to go HIGH
    absolute_time_t timeout = make_timeout_time_ms(30);

    while (gpio_get(echo_pin) == 0)
    {
        if (time_reached(timeout))
            return -1.0f;
    }

    // Start measuring ECHO pulse
    uint64_t start = time_us_64();

    timeout = make_timeout_time_ms(30);

    while (gpio_get(echo_pin) == 1)
    {
        if (time_reached(timeout))
            return -1.0f;
    }

    uint64_t end = time_us_64();

    // Calculate distance
    float duration = (float)(end - start);
    float distance = (duration * 0.0343f) / 2.0f;

    return distance;
}

int main()
{
    stdio_init_all();

    // Initialize all three LEDs
    init_led(LED1);
    init_led(LED2);
    init_led(LED3);

    // Initialize all three ultrasonic sensors
    init_ultrasonic(TRIG1, ECHO1);
    init_ultrasonic(TRIG2, ECHO2);
    init_ultrasonic(TRIG3, ECHO3);

    while (true)
    {
        // Sensor 1 controls LED 1
        float distance1 = get_distance_cm(TRIG1, ECHO1);

        gpio_put(LED1,
            distance1 > 0 && distance1 <= DETECT_DISTANCE_CM);

        sleep_ms(60);

        // Sensor 2 controls LED 2
        float distance2 = get_distance_cm(TRIG2, ECHO2);

        gpio_put(LED2,
            distance2 > 0 && distance2 <= DETECT_DISTANCE_CM);

        sleep_ms(60);

        // Sensor 3 controls LED 3
        float distance3 = get_distance_cm(TRIG3, ECHO3);

        gpio_put(LED3,
            distance3 > 0 && distance3 <= DETECT_DISTANCE_CM);

        sleep_ms(60);

        // Print measurements
        printf("S1: %.2f cm | S2: %.2f cm | S3: %.2f cm\n",
               distance1, distance2, distance3);
    }
}
