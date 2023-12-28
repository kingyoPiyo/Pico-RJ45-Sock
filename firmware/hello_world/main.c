/********************************************************
* Title    : Pico-10BASE-T Sample
* Date     : 2022/08/22
* Design   : kingyo
********************************************************/
#include <stdio.h>
#include "pico/stdlib.h"
#include "hwinit.h"
#include "eth.h"


int main() {
    stdio_init_all();
    hw_init();
    eth_init();

    printf("[BOOT]\r\n");

    hw_start_led_blink();

    while (1) {
        eth_main();
    }
}
