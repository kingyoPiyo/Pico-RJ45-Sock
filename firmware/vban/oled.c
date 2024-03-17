/*
I am referring to the pico-example project.
https://github.com/raspberrypi/pico-examples/tree/master/i2c/ssd1306_i2c
The following is the line sense clause below.
----------------
Copyright 2020 (c) 2020 Raspberry Pi (Trading) Ltd.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following
   disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following
   disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "system.h"
#include "oled.h"


const static uint8_t oled_ptn[] = {
    127,64,64,64,0,0,0,85,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,65,0,85,
    248,72,72,176,0,0,0,168,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,168,
    3,0,0,3,0,0,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,
    254,18,18,12,0,0,0,170,0,130,0,130,0,130,0,130,0,130,0,130,0,130,0,130,0,130,0,130,0,130,0,130,0,130,0,130,0,130,0,130,0,130,0,130,0,130,0,130,0,130,0,130,0,130,0,130,0,130,0,130,0,130,0,130,0,130,0,130,0,130,0,170,0,0,132,254,128,0,28,162,162,124,0,196,162,146,140,0,128,0,132,254,128,0,124,146,146,96,0,108,146,146,108,0,128,0,68,130,146,108,0,194,34,18,14,0,128,0,196,162,146,140,0,24,20,18,254,0
};


// Define the size of the display we have attached. This can vary, make sure you
// have the right size defined or the output will look rather odd!
// Code has been tested on 128x32 and 128x64 OLED displays
#define SSD1306_HEIGHT              32
#define SSD1306_WIDTH               128

#define SSD1306_I2C_ADDR            _u(0x3C)

// 400 is usual, but often these can be overclocked to improve display response.
// Tested at 1000 on both 32 and 84 pixel height devices and it worked.
//#define SSD1306_I2C_CLK             400
#define SSD1306_I2C_CLK             1000


// commands (see datasheet)
#define SSD1306_SET_MEM_MODE        _u(0x20)
#define SSD1306_SET_COL_ADDR        _u(0x21)
#define SSD1306_SET_PAGE_ADDR       _u(0x22)
#define SSD1306_SET_HORIZ_SCROLL    _u(0x26)
#define SSD1306_SET_SCROLL          _u(0x2E)

#define SSD1306_SET_DISP_START_LINE _u(0x40)

#define SSD1306_SET_CONTRAST        _u(0x81)
#define SSD1306_SET_CHARGE_PUMP     _u(0x8D)

#define SSD1306_SET_SEG_REMAP       _u(0xA0)
#define SSD1306_SET_ENTIRE_ON       _u(0xA4)
#define SSD1306_SET_ALL_ON          _u(0xA5)
#define SSD1306_SET_NORM_DISP       _u(0xA6)
#define SSD1306_SET_INV_DISP        _u(0xA7)
#define SSD1306_SET_MUX_RATIO       _u(0xA8)
#define SSD1306_SET_DISP            _u(0xAE)
#define SSD1306_SET_COM_OUT_DIR     _u(0xC0)
#define SSD1306_SET_COM_OUT_DIR_FLIP _u(0xC0)

#define SSD1306_SET_DISP_OFFSET     _u(0xD3)
#define SSD1306_SET_DISP_CLK_DIV    _u(0xD5)
#define SSD1306_SET_PRECHARGE       _u(0xD9)
#define SSD1306_SET_COM_PIN_CFG     _u(0xDA)
#define SSD1306_SET_VCOM_DESEL      _u(0xDB)

#define SSD1306_PAGE_HEIGHT         _u(8)
#define SSD1306_NUM_PAGES           (SSD1306_HEIGHT / SSD1306_PAGE_HEIGHT)
#define SSD1306_BUF_LEN             (SSD1306_NUM_PAGES * SSD1306_WIDTH)

#define SSD1306_WRITE_MODE         _u(0xFE)
#define SSD1306_READ_MODE          _u(0xFF)


struct render_area {
    uint8_t start_col;
    uint8_t end_col;
    uint8_t start_page;
    uint8_t end_page;

    int buflen;
};

static uint8_t buf[SSD1306_BUF_LEN];

struct render_area frame_area = {
    start_col: 0,
    end_col : SSD1306_WIDTH - 1,
    start_page : 0,
    end_page : SSD1306_NUM_PAGES - 1
    };


void calc_render_area_buflen(struct render_area *area) {
    // calculate how long the flattened buffer will be for a render area
    area->buflen = (area->end_col - area->start_col + 1) * (area->end_page - area->start_page + 1);
}

void SSD1306_send_cmd(uint8_t cmd) {
    // I2C write process expects a control byte followed by data
    // this "data" can be a command or data to follow up a command
    // Co = 1, D/C = 0 => the driver expects a command
    uint8_t buf[2] = {0x80, cmd};
    i2c_write_blocking(i2c1, SSD1306_I2C_ADDR, buf, 2, false);
}

void SSD1306_send_cmd_list(uint8_t *buf, int num) {
    for (int i=0;i<num;i++)
        SSD1306_send_cmd(buf[i]);
}

void SSD1306_send_buf(uint8_t buf[], int buflen) {
    uint8_t *temp_buf = malloc(buflen + 1);

    temp_buf[0] = 0x40;
    memcpy(temp_buf+1, buf, buflen);

    i2c_write_blocking(i2c1, SSD1306_I2C_ADDR, temp_buf, buflen + 1, false);

    free(temp_buf);
}


void SSD1306_init() {
    // Some of these commands are not strictly necessary as the reset
    // process defaults to some of these but they are shown here
    // to demonstrate what the initialization sequence looks like
    // Some configuration values are recommended by the board manufacturer

    uint8_t cmds[] = {
        SSD1306_SET_DISP,               // set display off
        /* memory mapping */
        SSD1306_SET_MEM_MODE,           // set memory address mode 0 = horizontal, 1 = vertical, 2 = page
        0x00,                           // horizontal addressing mode
        /* resolution and layout */
        SSD1306_SET_DISP_START_LINE,    // set display start line to 0
        SSD1306_SET_SEG_REMAP | 0x01,   // set segment re-map, column address 127 is mapped to SEG0
        SSD1306_SET_MUX_RATIO,          // set multiplex ratio
        SSD1306_HEIGHT - 1,             // Display height - 1
        SSD1306_SET_COM_OUT_DIR | 0x08, // set COM (common) output scan direction. Scan from bottom up, COM[N-1] to COM0
        SSD1306_SET_DISP_OFFSET,        // set display offset
        0x00,                           // no offset
        SSD1306_SET_COM_PIN_CFG,        // set COM (common) pins hardware configuration. Board specific magic number. 
                                        // 0x02 Works for 128x32, 0x12 Possibly works for 128x64. Other options 0x22, 0x32
#if ((SSD1306_WIDTH == 128) && (SSD1306_HEIGHT == 32))
        0x02,                           
#elif ((SSD1306_WIDTH == 128) && (SSD1306_HEIGHT == 64))
        0x12,
#else
        0x02,
#endif
        /* timing and driving scheme */
        SSD1306_SET_DISP_CLK_DIV,       // set display clock divide ratio
        0x80,                           // div ratio of 1, standard freq
        SSD1306_SET_PRECHARGE,          // set pre-charge period
        0xF1,                           // Vcc internally generated on our board
        SSD1306_SET_VCOM_DESEL,         // set VCOMH deselect level
        0x30,                           // 0.83xVcc
        /* display */
        SSD1306_SET_CONTRAST,           // set contrast control
        0xFF,
        SSD1306_SET_ENTIRE_ON,          // set entire display on to follow RAM content
        SSD1306_SET_NORM_DISP,           // set normal (not inverted) display
        SSD1306_SET_CHARGE_PUMP,        // set charge pump
        0x14,                           // Vcc internally generated on our board
        SSD1306_SET_SCROLL | 0x00,      // deactivate horizontal scrolling if set. This is necessary as memory writes will corrupt if scrolling was enabled
        SSD1306_SET_DISP | 0x01, // turn display on
    };

    SSD1306_send_cmd_list(cmds, count_of(cmds));
}


void render(uint8_t *buf, struct render_area *area) {
    // update a portion of the display with a render area
    uint8_t cmds[] = {
        SSD1306_SET_COL_ADDR,
        area->start_col,
        area->end_col,
        SSD1306_SET_PAGE_ADDR,
        area->start_page,
        area->end_page
    };
    
    SSD1306_send_cmd_list(cmds, count_of(cmds));
    SSD1306_send_buf(buf, area->buflen);
}

static void SetPixel(uint8_t *buf, int x,int y, bool on) {
    assert(x >= 0 && x < SSD1306_WIDTH && y >=0 && y < SSD1306_HEIGHT);

    const int BytesPerRow = SSD1306_WIDTH ; // x pixels, 1bpp, but each row is 8 pixel high, so (x / 8) * 8

    int byte_idx = (y / 8) * BytesPerRow + x;
    uint8_t byte = buf[byte_idx];

    if (on)
        byte |=  1 << (y % 8);
    else
        byte &= ~(1 << (y % 8));

    buf[byte_idx] = byte;
}
// Basic Bresenhams.
static void DrawLine(uint8_t *buf, int x0, int y0, int x1, int y1, bool on) {
    int dx =  abs(x1-x0);
    int sx = x0<x1 ? 1 : -1;
    int dy = -abs(y1-y0);
    int sy = y0<y1 ? 1 : -1;
    int err = dx+dy;
    int e2;

    while (true) {
        SetPixel(buf, x0, y0, on);
        if (x0 == x1 && y0 == y1)
            break;
        e2 = 2*err;

        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void oled_init(void)
{
    // I2C is "open drain", pull ups to keep signal high when no data is being
    // sent
    i2c_init(i2c1, SSD1306_I2C_CLK * 1000);
    gpio_set_function(DEF_SYS_HWPIN_OLED_SDA, GPIO_FUNC_I2C);
    gpio_set_function(DEF_SYS_HWPIN_OLED_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(DEF_SYS_HWPIN_OLED_SDA);
    gpio_pull_up(DEF_SYS_HWPIN_OLED_SCL);

    // run through the complete initialization process
    SSD1306_init();

    // Initialize render area for entire frame (SSD1306_WIDTH pixels by SSD1306_NUM_PAGES pages)
    calc_render_area_buflen(&frame_area);

    // 背景
    for (uint32_t i = 0; i < SSD1306_BUF_LEN; i++) {
        buf[i] = oled_ptn[i];
    }
    render(buf, &frame_area);
}


void oled_main(uint8_t lv_l, uint8_t lv_r, uint8_t pkt_rate)
{
    static uint8_t pk_l = 0, pk_r = 0; // ピークレベル
    static uint8_t cnt = 0;

    // 背景
    for (uint32_t i = 0; i < SSD1306_BUF_LEN; i++) {
        buf[i] = oled_ptn[i];
    }
    
    // バーグラフ生成
    DrawLine(buf, 7, 0, lv_l + 7, 0, true);
    DrawLine(buf, 7, 1, lv_l + 7, 1, true);
    DrawLine(buf, 7, 2, lv_l + 7, 2, true);
    DrawLine(buf, 7, 3, lv_l + 7, 3, true);
    DrawLine(buf, 7, 4, lv_l + 7, 4, true);
    DrawLine(buf, 7, 5, lv_l + 7, 5, true);
    DrawLine(buf, 7, 6, lv_l + 7, 6, true);

    DrawLine(buf, 7, 11, lv_r + 7, 11, true);
    DrawLine(buf, 7, 12, lv_r + 7, 12, true);
    DrawLine(buf, 7, 13, lv_r + 7, 13, true);
    DrawLine(buf, 7, 14, lv_r + 7, 14, true);
    DrawLine(buf, 7, 15, lv_r + 7, 15, true);
    DrawLine(buf, 7, 16, lv_r + 7, 16, true);
    DrawLine(buf, 7, 17, lv_r + 7, 17, true);

    DrawLine(buf, 7, 25, pkt_rate + 7, 25, true);
    DrawLine(buf, 7, 26, pkt_rate + 7, 26, true);
    DrawLine(buf, 7, 27, pkt_rate + 7, 27, true);
    DrawLine(buf, 7, 28, pkt_rate + 7, 28, true);
    DrawLine(buf, 7, 29, pkt_rate + 7, 29, true);
    DrawLine(buf, 7, 30, pkt_rate + 7, 30, true);
    DrawLine(buf, 7, 31, pkt_rate + 7, 31, true);

    // ピーク表示
    if (lv_l > pk_l) {
        pk_l = lv_l;
    } else {
        if (cnt % 5 == 0) pk_l = pk_l > 0 ? pk_l-1 : 0;
    }

    if (lv_r > pk_r) {
        pk_r = lv_r;
    } else {
        if (cnt % 5 == 0) pk_r = pk_r > 0 ? pk_r-1 : 0;
    }

    DrawLine(buf, pk_l + 7, 0, pk_l + 7, 6, true);
    DrawLine(buf, pk_r + 7, 11, pk_r + 7, 17, true);
    cnt++;

    render(buf, &frame_area);
}
