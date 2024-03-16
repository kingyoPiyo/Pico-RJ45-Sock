/********************************************************
* Title    : Pico-10BASE-T VBAN Sample
* Date     : 2024/03/15
* Design   : kingyo
********************************************************/
#include <stdio.h>
#include "pico/stdlib.h"
#include "hwinit.h"
#include "eth.h"
#include "vban.h"


int main() {
    stdio_init_all();
    hw_init();
    eth_init();
    vban_init();

    printf("[BOOT]\r\n");

    hw_start_led_blink();

    while (1) {
        eth_main();
        vban_main();
    }
}
