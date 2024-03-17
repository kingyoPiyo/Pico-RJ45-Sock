#ifndef __SYSTEM_H__
#define __SYSTEM_H__

// Compile switch
#define UART_EBG_EN             (1)     // 有効にするとちょい重たい
#define FCS_DMA_EN              (1)     // FCSの計算にDMAを使用する
#define DEF_10BASET_FULL_EN     (1)     // Enable 10BASE-T Full Duplex


// RasPico Network settings
#define DEF_SYS_PICO_MAC        (0x123456789ABC)

#define DEF_SYS_PICO_IP1        (192)
#define DEF_SYS_PICO_IP2        (168)
#define DEF_SYS_PICO_IP3        (37)
#define DEF_SYS_PICO_IP4        (24)


// For UDP
#define DEF_SYS_UDP_DST_MAC     (0xFFFFFFFFFFFF)

#define DEF_SYS_UDP_DST_IP1     (192)
#define DEF_SYS_UDP_DST_IP2     (168)
#define DEF_SYS_UDP_DST_IP3     (37)
#define DEF_SYS_UDP_DST_IP4     (19)

// H/W PIN
#define DEF_SYS_HWPIN_ADC0      (26)
#define DEF_SYS_HWPIN_ADC1      (27)
#define DEF_SYS_HWPIN_DCDC_PS   (23)
#define DEF_SYS_HWPIN_OLED_SCL  (11)
#define DEF_SYS_HWPIN_OLED_SDA  (10)

#endif //__SYSTEM_H__
