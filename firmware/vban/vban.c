/********************************************************
* Title    : VBAN.c
* Date     : 2022/07/25
* Design   : kingyo
* Note     : ADC0 -- Audio L-Ch
             ADC1 -- Audio R-Ch
********************************************************/
#include <stdlib.h>
#include "vban.h"
#include "udp.h"
#include "eth.h"
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "hardware/irq.h"
#include "hardware/adc.h"
#include "ser_10base_t.pio.h"
#include "system.h"

#define VBAN_X4_OVERSAMPL_ON    // x4 Oversampling

#define VBAN_SR_MAXNUMBER           21
#define VBAN_PROTOCOL_AUDIO         0x00
#define VBAN_PROTOCOL_SERIAL        0x20
#define VBAN_PROTOCOL_TXT           0x40
#define VBAN_PROTOCOL_SERVICE       0x60
#define VBAN_PROTOCOL_UNDEFINED_1   0x80
#define VBAN_PROTOCOL_UNDEFINED_2   0xA0
#define VBAN_PROTOCOL_UNDEFINED_3   0xC0
#define VBAN_PROTOCOL_USER          0xE0

#define VBAN_DATATYPE_BYTE8         0x00
#define VBAN_DATATYPE_INT16         0x01
#define VBAN_DATATYPE_INT24         0x02
#define VBAN_DATATYPE_INT32         0x03
#define VBAN_DATATYPE_FLOAT32       0x04
#define VBAN_DATATYPE_FLOAT64       0x05
#define VBAN_DATATYPE_12BITS        0x06
#define VBAN_DATATYPE_10BITS        0x07

#define VBAN_CODEC_PCM              0x00
#define VBAN_CODEC_VBCA             0x10
#define VBAN_CODEC_VBCV             0x20
#define VBAN_CODEC_UNDEFINED_1      0x30
#define VBAN_CODEC_UNDEFINED_2      0x40
#define VBAN_CODEC_UNDEFINED_3      0x50
#define VBAN_CODEC_UNDEFINED_4      0x60
#define VBAN_CODEC_UNDEFINED_5      0x70
#define VBAN_CODEC_UNDEFINED_6      0x80
#define VBAN_CODEC_UNDEFINED_7      0x90
#define VBAN_CODEC_UNDEFINED_8      0xA0
#define VBAN_CODEC_UNDEFINED_9      0xB0
#define VBAN_CODEC_UNDEFINED_10     0xC0
#define VBAN_CODEC_UNDEFINED_11     0xD0
#define VBAN_CODEC_UNDEFINED_12     0xE0
#define VBAN_CODEC_USER             0xF0


static char stream_name[16] = "Stream1";
static uint32_t tx_buf_udp[DEF_UDP_BUF_SIZE+1] = {0};

static uint32_t dma_ch_100base_fx;

static uint8_t sel = 0;

typedef struct {
    T_VBAN_HEADER header;
    int16_t pcm[DEF_VBAN_PCM_SIZE/2];
} vban_payload_t;

static vban_payload_t vban_payload;

// ADC Buffer
#define DEF_BUF_NUM     (32)    // リングバッファサイズ
static uint32_t buf_wp = 0;     // 書き込みポインタ
static uint32_t buf_rp = 0;     // 読み出しポインタ
static int16_t adc_buf[DEF_BUF_NUM][DEF_VBAN_PCM_SIZE/2];


// ADC Conversion Interrupt
static void __time_critical_func(adc_irq_handler) (void)
{
    static uint8_t lp = 0;
    static uint8_t os = 0;
    static int16_t l_tmp = 0;
    static int16_t r_tmp = 0;

#ifdef VBAN_X4_OVERSAMPL_ON
    // x4 oversampling
    switch (os) {
        case 0:
            l_tmp = adc_fifo_get(); os++; break;
        case 1:
            r_tmp = adc_fifo_get(); os++; break;
        case 2:
            l_tmp += adc_fifo_get(); os++; break;
        case 3:
            r_tmp += adc_fifo_get(); os++; break;
        case 4:
            l_tmp += adc_fifo_get(); os++; break;
        case 5:
            r_tmp += adc_fifo_get(); os++; break;
        case 6:
            l_tmp += adc_fifo_get(); os++; break;
        case 7:
            r_tmp += adc_fifo_get(); os = 0;
            adc_buf[buf_wp][lp++] = (l_tmp - 8192) * 4;
            adc_buf[buf_wp][lp++] = (r_tmp - 8192) * 4;
            if (lp == 0) {
                buf_wp = (buf_wp + 1) % DEF_BUF_NUM;
            }
            break;
    }
#else
    adc_buf[buf_wp][lp++] = (adc_fifo_get() - 2048) * 8;
    if (lp == 0) {
        buf_wp = (buf_wp + 1) % DEF_BUF_NUM;
    }
#endif
}


void vban_init(void)
{
    // VBAN Header
    vban_payload.header.vban = ('N' << 24) + ('A' << 16) + ('B' << 8) + ('V' << 0);
    vban_payload.header.format_SR = 16;      // 44.1kHz
    vban_payload.header.format_nbs = 128-1;  // 128 Samples/frame
    vban_payload.header.format_nbc = 2-1;    // 2CH (Stereo)
    vban_payload.header.format_bit = VBAN_DATATYPE_INT16;   // Bit resolution (16bit)
    for (uint8_t i = 0; i < 16; i++) {
        vban_payload.header.streamname[i] = stream_name[i];
    }
    vban_payload.header.nuFrame = 0;

    // バッファ初期化
    buf_wp = buf_rp = 0;

    // ADC
    adc_select_input(0);                        // Start is ADC0(L-Ch)
#ifdef VBAN_X4_OVERSAMPL_ON
    adc_set_clkdiv((48000000.0/(44100*8))-1.0); // 44.1kHz x 4 * 2CH = 352.8kS/s Round-Robin
#else
    adc_set_clkdiv((48000000.0/(44100*2))-1.0); // 44.1kHz * 2CH = 88.2kS/s Round-Robin
#endif
    adc_set_round_robin(0b00000011);            // ADC0 & ADC1

    // Set IRQ handler
    irq_add_shared_handler(ADC_IRQ_FIFO, &adc_irq_handler, 128);
    irq_set_enabled(ADC_IRQ_FIFO, true);

    // ADC FIFO
    adc_fifo_setup	(
        true,       // Enables write each conversion result to the FIFO
        false,      // Enable DMA requests when FIFO contains data
        1,          // Threshold for DMA requests/FIFO IRQ if enabled.
        false,      // If enabled, bit 15 of the FIFO contains error flag for each sample
        false       // Shift FIFO contents to be one byte in size (for byte DMA) - enables DMA to byte buffers.
    );

    adc_irq_set_enabled(true);
    irq_set_priority(ADC_IRQ_FIFO, 0);
    sleep_ms(1);    // A nap will help you concentrate.
    adc_run(true);  // ADC Free running start!
}

uint32_t vban_main(uint8_t *lv_l, uint8_t *lv_r, bool max_rst)
{
    static uint32_t tmp_l = 0, tmp_r = 0;

    if (max_rst) { tmp_l = 0; tmp_r = 0; }
    if (buf_wp == buf_rp) return 0;
    
    // Increment VBAN Frame counter
    vban_payload.header.nuFrame++;

    // Copy PCM data
    for (uint32_t i = 0; i < (DEF_VBAN_PCM_SIZE/2); i++) {
        vban_payload.pcm[i] = adc_buf[buf_rp][i];
        // For Level Meter, Max hold
        if (i & 0x01) {
            if (abs(adc_buf[buf_rp][i]) > tmp_r) tmp_r = abs(adc_buf[buf_rp][i]); // R
        } else {
            if (abs(adc_buf[buf_rp][i]) > tmp_l) tmp_l = abs(adc_buf[buf_rp][i]); // L
        }
    }
    udp_packet_gen_10base(tx_buf_udp, (uint8_t *)&vban_payload);
    eth_tx_data(tx_buf_udp, DEF_UDP_BUF_SIZE+1);
    buf_rp = (buf_rp + 1) % DEF_BUF_NUM;

    // 暫定
    // ((4095 * 4) - 8192) * 4 / 120
    *lv_l = (uint8_t)(tmp_l / 273);
    *lv_r = (uint8_t)(tmp_r / 273);

    return 1;
}
