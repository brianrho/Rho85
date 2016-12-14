#ifndef TFT_SCREEN_H
#define TFT_SCREEN_H

#include <xc.h>
#include <stdint.h>
#include "utils.h"

#define TFT_CLK_DIR TRISBbits.RB3
#define TFT_CS_DIR TRISBbits.RB6
#define TFT_SDA_DIR TRISBbits.RB5
#define TFT_RST_DIR TRISBbits.RB4
#define TFT_RS_DIR TRISBbits.RB7

#define TFT_CLK LATBbits.LATB3
#define TFT_CS LATBbits.LATB6
#define TFT_SDA LATBbits.LATB5
#define TFT_RST LATBbits.LATB4
#define TFT_RS LATBbits.LATB7

#define ST7735_TFTWIDTH  128
#define ST7735_TFTHEIGHT  160

#define ST7735_NOP     0x00
#define ST7735_SWRESET 0x01
#define ST7735_RDDID   0x04
#define ST7735_RDDST   0x09

#define ST7735_SLPIN   0x10
#define ST7735_SLPOUT  0x11
#define ST7735_PTLON   0x12
#define ST7735_NORON   0x13

#define ST7735_INVOFF  0x20
#define ST7735_INVON   0x21
#define ST7735_DISPOFF 0x28
#define ST7735_DISPON  0x29
#define ST7735_CASET   0x2A
#define ST7735_RASET   0x2B
#define ST7735_RAMWR   0x2C
#define ST7735_RAMRD   0x2E

#define ST7735_PTLAR   0x30
#define ST7735_COLMOD  0x3A
#define ST7735_MADCTL  0x36

#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR  0xB4
#define ST7735_DISSET5 0xB6

#define ST7735_PWCTR1  0xC0
#define ST7735_PWCTR2  0xC1
#define ST7735_PWCTR3  0xC2
#define ST7735_PWCTR4  0xC3
#define ST7735_PWCTR5  0xC4
#define ST7735_VMCTR1  0xC5

#define ST7735_RDID1   0xDA
#define ST7735_RDID2   0xDB
#define ST7735_RDID3   0xDC
#define ST7735_RDID4   0xDD

#define ST7735_PWCTR6  0xFC

#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

// Color definitions
#define	ST7735_BLACK   0x0000
#define	ST7735_RED     0x001F
#define	ST7735_BLUE    0xF800
#define	ST7735_GREEN   0x07E0
#define ST7735_CYAN    0x07FF
#define ST7735_MAGENTA 0xF81F
#define ST7735_YELLOW  0xFFE0
#define ST7735_WHITE   0xFFFF

void init_screen(void);
void set_window(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
void push_color(uint16_t color);
void fill_screen(uint16_t color);
void draw_pixel(int16_t x, int16_t y, uint16_t color);
void draw_vline(int16_t x, int16_t y, int16_t h, uint16_t color);
void draw_hline(int16_t x, int16_t y, int16_t w, uint16_t color);
void draw_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void draw_round_rect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
void writech(uint8_t c);
void draw_char(int16_t x, int16_t y, unsigned char c, uint16_t color, uint8_t size);
void draw_bitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color);
void set_rotation(uint8_t r);
void set_cursor(int16_t x, int16_t y);
void set_text_color(uint16_t c);
uint16_t color_565(uint8_t r, uint8_t g, uint8_t b);
void set_text_size(uint8_t s);

#endif