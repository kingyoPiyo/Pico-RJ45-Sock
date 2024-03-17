#ifndef __OLED_H__
#define __OLED_H__

#include <stdint.h>

void oled_init(void);
void oled_main(uint8_t lv_l, uint8_t lv_r, uint8_t pkt_rate);

#endif //__OLED_H__
