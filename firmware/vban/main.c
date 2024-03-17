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
#include "oled.h"

int main() {
    uint8_t lv_l = 0, lv_r = 0;
    uint32_t lp_cnt = 0;
    uint32_t packet_cnt = 0;
    bool max_rst = true;

    stdio_init_all();
    hw_init();
    oled_init();
    eth_init();
    vban_init();
    
    printf("[BOOT]\r\n");

    hw_start_led_blink();

    uint32_t time = time_us_32();

    while (1) {
        packet_cnt += eth_main();
        packet_cnt += vban_main(&lv_l, &lv_r, max_rst);

        // 40ms Interval
        if (time_us_32() - time > 40000) {
            time = time_us_32();
            packet_cnt = packet_cnt / 2;
            if (packet_cnt > 64) packet_cnt = 64;
            oled_main(lv_l, lv_r, (uint8_t)packet_cnt);
            packet_cnt = 0;
            max_rst = true;
        } else {
            max_rst = false;
        }

        lp_cnt++;
    }
}
