#include <stdio.h>
#include "pico/stdlib.h"

int main() {

    gpio_init(0);          // directly use pin 0
    gpio_set_dir(0, GPIO_OUT);

    while (true) {
        gpio_put(0, 1);    // turn GPIO0 high (LED on if wired active-high)
        sleep_ms(50000);
        gpio_put(0, 0);    // turn GPIO0 low (LED off)
        sleep_ms(50000);
    }
}