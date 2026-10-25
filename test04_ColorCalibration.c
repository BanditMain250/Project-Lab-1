#include <stdio.h>
#include "pico/stdlib.h"w
#include "hardware/gpio.h"

// LED pins
#define RED_LED       16
#define GREEN_LED     17
#define YELLOW_LED    18

// TCS3200 color sensor pins
#define S0_PIN        15
#define S1_PIN        14
#define S2_PIN        13
#define S3_PIN        12

// Additional connection required
#define OUT_PIN       19


#define SAMPLE_MS     50

// Pulse counter
volatile uint32_t pulse_count = 0;

// White calibration values
float white_r = 1.0f;
float white_g = 1.0f;
float white_b = 1.0f;
bool calibrated = false;

// Count pulses from TCS3200
void pulse_callback(uint gpio, uint32_t events)
{
    if (gpio == OUT_PIN && (events & GPIO_IRQ_EDGE_RISE)) {
        pulse_count++;
    }
}

// Select the sensor's color filter
void select_filter(int color)
{
    switch (color) {
        case 0: // Red
            gpio_put(S2_PIN, 0);
            gpio_put(S3_PIN, 0);
            break;

        case 1: // Green
            gpio_put(S2_PIN, 1);
            gpio_put(S3_PIN, 1);
            break;

        case 2: // Blue
            gpio_put(S2_PIN, 0);
            gpio_put(S3_PIN, 1);
            break;
    }
}

// Measure output frequency
uint32_t read_color(int color)
{
    select_filter(color);

    // Allow sensor output to settle
    sleep_ms(5);

    gpio_set_irq_enabled(OUT_PIN, GPIO_IRQ_EDGE_RISE, false);
    pulse_count = 0;
    gpio_set_irq_enabled(OUT_PIN, GPIO_IRQ_EDGE_RISE, true);

    sleep_ms(SAMPLE_MS);

    gpio_set_irq_enabled(OUT_PIN, GPIO_IRQ_EDGE_RISE, false);

    return pulse_count;
}

// Read all three color channels
void read_rgb(uint32_t *r, uint32_t *g, uint32_t *b)
{
    *r = read_color(0);
    *g = read_color(1);
    *b = read_color(2);
}

// Turn off all LEDs
void leds_off(void)
{
    gpio_put(RED_LED, 0);
    gpio_put(GREEN_LED, 0);
    gpio_put(YELLOW_LED, 0);
}

// Calibrate using a white surface
void calibrate_white(void)
{
    uint32_t r, g, b;

    printf("\nPlace WHITE surface under sensor.\n");
    printf("Calibrating in 3 seconds...\n");

    leds_off();
    sleep_ms(3000);

    read_rgb(&r, &g, &b);

    if (r == 0 || g == 0 || b == 0) {
        calibrated = false;
        printf("Calibration failed: check sensor wiring.\n");
        return;
    }

    white_r = (float)r;
    white_g = (float)g;
    white_b = (float)b;

    calibrated = true;

    printf("Calibration complete!\n");
    printf("White R=%lu G=%lu B=%lu\n",
           (unsigned long)r,
           (unsigned long)g,
           (unsigned long)b);
}

int main()
{
    stdio_init_all();

    // Initialize LEDs
    const uint leds[] = {RED_LED, GREEN_LED, YELLOW_LED};

    for (int i = 0; i < 3; i++) {
        gpio_init(leds[i]);
        gpio_set_dir(leds[i], GPIO_OUT);
        gpio_put(leds[i], 0);
    }

    // Initialize TCS3200 control pins
    const uint controls[] = {S0_PIN, S1_PIN, S2_PIN, S3_PIN};

    for (int i = 0; i < 4; i++) {
        gpio_init(controls[i]);
        gpio_set_dir(controls[i], GPIO_OUT);
    }

    // TCS3200 output scaling = 20%
    gpio_put(S0_PIN, 1);
    gpio_put(S1_PIN, 0);

    // Initialize sensor OUT input
    gpio_init(OUT_PIN);
    gpio_set_dir(OUT_PIN, GPIO_IN);

    gpio_set_irq_enabled_with_callback(
        OUT_PIN,
        GPIO_IRQ_EDGE_RISE,
        false,
        &pulse_callback
    );

    sleep_ms(2000);

    printf("\nTCS3200 COLOR SENSOR READY\n");
    printf("Enter 'w' to calibrate on white.\n");

    while (true) {

        // Check serial input for calibration
        int command = getchar_timeout_us(0);

        if (command == 'w' || command == 'W') {
            calibrate_white();
        }

        if (!calibrated) {
            leds_off();
            sleep_ms(100);
            continue;
        }

        uint32_t r, g, b;

        read_rgb(&r, &g, &b);

        // Normalize readings against white calibration
        float R = r / white_r;
        float G = g / white_g;
        float B = b / white_b;

        // Start with all LEDs off
        leds_off();

        const char *detected = "UNKNOWN";

        // Require sufficient reflected light
        float brightest = R;

        if (G > brightest) brightest = G;
        if (B > brightest) brightest = B;

        if (brightest >= 0.20f) {

            // MAGENTA: Red + Blue, weak Green
            if (G < R * 0.65f &&
                G < B * 0.65f &&
                R > 0.35f &&
                B > 0.35f) {

                gpio_put(RED_LED, 1);
                detected = "MAGENTA";
            }

            // CYAN: Green + Blue, weak Red
            else if (R < G * 0.65f &&
                     R < B * 0.65f &&
                     G > 0.35f &&
                     B > 0.35f) {

                gpio_put(GREEN_LED, 1);
                detected = "CYAN";
            }

            // YELLOW: Red + Green, weak Blue
            else if (B < R * 0.65f &&
                     B < G * 0.65f &&
                     R > 0.35f &&
                     G > 0.35f) {

                gpio_put(YELLOW_LED, 1);
                detected = "YELLOW";
            }
        }

        printf("R: %.2f G: %.2f B: %.2f | %s\n",
               R, G, B, detected);

        sleep_ms(100);
    }
}