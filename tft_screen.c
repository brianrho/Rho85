#include "tft_screen.h"
#include "gfxfont.h"

#ifndef _swap_int16_t
#define _swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }
#endif

static uint8_t _width    = ST7735_TFTWIDTH;
static uint8_t _height   = ST7735_TFTHEIGHT;
static uint8_t rotation  = 1;
static uint8_t cursor_y  = 0;
static uint8_t cursor_x  = 0;
static uint8_t textsize  = 1;
static uint16_t textcolor = 0xFFFF;
static uint8_t text_wrap      = 1;
static uint8_t colstart = 0, rowstart = 0;

void spi_write(uint8_t val){
    uint8_t bits;
    for (bits = 0x80; bits; bits >>= 1){
        if (val & bits) TFT_SDA = 1;
        else TFT_SDA = 0;
        TFT_CLK = 1;
        TFT_CLK = 0;
    }
}

void write_command(uint8_t val){
    TFT_RS = 0;
    TFT_CS = 0;
    spi_write(val);
    TFT_CS = 1;
}

void write_data(uint8_t val){
    TFT_RS = 1;
    TFT_CS = 0;
    spi_write(val);
    TFT_CS = 1;
}

#define DELAY 0x80
static const uint8_t Rcmd1[] = {                 // Init for 7735R, part 1 (red or green tab)
    15,                       // 15 commands in list:
    ST7735_SWRESET,   DELAY,  //  1: Software reset, 0 args, w/delay
      150,                    //     150 ms delay
    ST7735_SLPOUT ,   DELAY,  //  2: Out of sleep mode, 0 args, w/delay
      255,                    //     500 ms delay
    ST7735_FRMCTR1, 3      ,  //  3: Frame rate ctrl - normal mode, 3 args:
      0x01, 0x2C, 0x2D,       //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    ST7735_FRMCTR2, 3      ,  //  4: Frame rate control - idle mode, 3 args:
      0x01, 0x2C, 0x2D,       //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    ST7735_FRMCTR3, 6      ,  //  5: Frame rate ctrl - partial mode, 6 args:
      0x01, 0x2C, 0x2D,       //     Dot inversion mode
      0x01, 0x2C, 0x2D,       //     Line inversion mode
    ST7735_INVCTR , 1      ,  //  6: Display inversion ctrl, 1 arg, no delay:
      0x07,                   //     No inversion
    ST7735_PWCTR1 , 3      ,  //  7: Power control, 3 args, no delay:
      0xA2,
      0x02,                   //     -4.6V
      0x84,                   //     AUTO mode
    ST7735_PWCTR2 , 1      ,  //  8: Power control, 1 arg, no delay:
      0xC5,                   //     VGH25 = 2.4C VGSEL = -10 VGH = 3 * AVDD
    ST7735_PWCTR3 , 2      ,  //  9: Power control, 2 args, no delay:
      0x0A,                   //     Opamp current small
      0x00,                   //     Boost frequency
    ST7735_PWCTR4 , 2      ,  // 10: Power control, 2 args, no delay:
      0x8A,                   //     BCLK/2, Opamp current small & Medium low
      0x2A,  
    ST7735_PWCTR5 , 2      ,  // 11: Power control, 2 args, no delay:
      0x8A, 0xEE,
    ST7735_VMCTR1 , 1      ,  // 12: Power control, 1 arg, no delay:
      0x0E,
    ST7735_INVOFF , 0      ,  // 13: Don't invert display, no args, no delay
    ST7735_MADCTL , 1      ,  // 14: Memory access control (directions), 1 arg:
      0xC8,                   //     row addr/col addr, bottom to top refresh
    ST7735_COLMOD , 1      ,  // 15: set color mode, 1 arg, no delay:
      0x05 },                 //     16-bit color
  
  Rcmd2red[] = {              // Init for 7735R, part 2 (red tab only)
    2,                        //  2 commands in list:
    ST7735_CASET  , 4      ,  //  1: Column addr set, 4 args, no delay:
      0x00, 0x00,             //     XSTART = 0
      0x00, 0x7F,             //     XEND = 127
    ST7735_RASET  , 4      ,  //  2: Row addr set, 4 args, no delay:
      0x00, 0x00,             //     XSTART = 0
      0x00, 0x9F },           //     XEND = 159
      
  Rcmd3[] = {                 // Init for 7735R, part 3 (red or green tab)
    4,                        //  4 commands in list:
    ST7735_GMCTRP1, 16      , //  1: Magical unicorn dust, 16 args, no delay:
      0x02, 0x1c, 0x07, 0x12,
      0x37, 0x32, 0x29, 0x2d,
      0x29, 0x25, 0x2B, 0x39,
      0x00, 0x01, 0x03, 0x10,
    ST7735_GMCTRN1, 16      , //  2: Sparkles and rainbows, 16 args, no delay:
      0x03, 0x1d, 0x07, 0x06,
      0x2E, 0x2C, 0x29, 0x2D,
      0x2E, 0x2E, 0x37, 0x3F,
      0x00, 0x00, 0x02, 0x10,
    ST7735_NORON  ,    DELAY, //  3: Normal display on, no args, w/delay
      10,                     //     10 ms delay
    ST7735_DISPON ,    DELAY, //  4: Main screen turn on, no args w/delay
      100 };                  //     100 ms delay


// Companion code to the above tables.  Reads and issues
// a series of LCD commands stored in PROGMEM byte array.
void command_list(const uint8_t *addr) {

  uint8_t  numCommands, numArgs;
  uint16_t ms;

  numCommands = *addr++;   // Number of commands to follow
  while(numCommands--) {                 // For each command...
    write_command(*addr++); //   Read, issue command
    numArgs  = *addr++;    //   Number of args to follow
    ms       = numArgs & DELAY;          //   If hibit set, delay follows args
    numArgs &= ~DELAY;                   //   Mask out delay bit
    while(numArgs--) {                   //   For each argument...
      write_data(*addr++);  //     Read, issue argument
    }

    if(ms) {
      ms = *addr++; // Read post-command delay time (ms)
      if(ms == 255) ms = 500;     // If 255, delay for 500 ms
      delay_ms(ms);
    }
  }
}

void init_screen(void){
    TFT_CS_DIR = 0; TFT_CS = 1;
    TFT_RS_DIR = 0; TFT_RS = 1;
    TFT_SDA_DIR = 0; TFT_SDA = 0;
    TFT_CLK_DIR = 0; TFT_CLK = 0;
    TFT_RST_DIR = 0; TFT_RST = 1;
    
    TFT_CS = 0;
    delay_ms(50);
    TFT_RST = 0;
    delay_ms(50);
    TFT_RST = 1;
    delay_ms(50);

    command_list(Rcmd1);
    command_list(Rcmd2red);
    command_list(Rcmd3);
    
    write_command(ST7735_MADCTL);
    write_data(0xC8);
}

void set_window(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
    write_command(ST7735_CASET); // Column addr set
    write_data(0x00);
    write_data(x0+colstart);     // XSTART 
    write_data(0x00);
    write_data(x1+colstart);     // XEND
    
    write_command(ST7735_RASET); // Row addr set
    write_data(0x00);
    write_data(y0+rowstart);     // YSTART
    write_data(0x00);
    write_data(y1+rowstart);     // YEND
    
    write_command(ST7735_RAMWR); // write to RAM
}

void push_color(uint16_t color) {
    TFT_RS = 1;
    TFT_CS = 0;
    
    spi_write(color >> 8);
    spi_write(color);
    
    TFT_CS = 1;
}

void draw_pixel(int16_t x, int16_t y, uint16_t color) {
    
    if((x < 0) ||(x >= _width) || (y < 0) || (y >= _height)) return;
    set_window(x,y,x+1,y+1);
    
    TFT_RS = 1;
    TFT_CS = 0;
    
    spi_write(color >> 8);
    spi_write(color);
    
    TFT_CS = 1;
}

void draw_vline(int16_t x, int16_t y, int16_t h, uint16_t color) {

    // Rudimentary clipping
    if((x >= _width) || (y >= _height)) return;
    if((y+h-1) >= _height) h = _height-y;
    set_window(x, y, x, y+h-1);

    uint8_t hi = color >> 8, lo = color;
    
    TFT_RS = 1;
    TFT_CS = 0;
    while (h--) {
        spi_write(hi);
        spi_write(lo);
    }
    
    TFT_CS = 1;
}

void draw_hline(int16_t x, int16_t y, int16_t w, uint16_t color) {

    // Rudimentary clipping
    if((x >= _width) || (y >= _height)) return;
    if((x+w-1) >= _width)  w = _width-x;
    set_window(x, y, x+w-1, y);

    uint8_t hi = color >> 8, lo = color;

    TFT_RS = 1;
    TFT_CS = 0;
    while (w--) {
        spi_write(hi);
        spi_write(lo);
    }
    
    TFT_CS = 1;
}

void draw_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    draw_hline(x, y, w, color);
    draw_hline(x, y+h-1, w, color);
    draw_vline(x, y, h, color);
    draw_vline(x+w-1, y, h, color);
}

void draw_circle_helper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color) {
    int16_t f     = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x     = 0;
    int16_t y     = r;

    while (x<y) {
        if (f >= 0) {
          y--;
          ddF_y += 2;
          f     += ddF_y;
        }
        x++;
        ddF_x += 2;
        f     += ddF_x;
        if (cornername & 0x4) {
          draw_pixel(x0 + x, y0 + y, color);
          draw_pixel(x0 + y, y0 + x, color);
        }
        if (cornername & 0x2) {
          draw_pixel(x0 + x, y0 - y, color);
          draw_pixel(x0 + y, y0 - x, color);
        }
        if (cornername & 0x8) {
          draw_pixel(x0 - y, y0 + x, color);
          draw_pixel(x0 - x, y0 + y, color);
        }
        if (cornername & 0x1) {
          draw_pixel(x0 - y, y0 - x, color);
          draw_pixel(x0 - x, y0 - y, color);
        }
    }
}

// Draw a rounded rectangle
void draw_round_rect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
  // smarter version
  draw_hline(x+r  , y    , w-2*r, color); // Top
  draw_hline(x+r  , y+h-1, w-2*r, color); // Bottom
  draw_vline(x    , y+r  , h-2*r, color); // Left
  draw_vline(x+w-1, y+r  , h-2*r, color); // Right
  // draw four corners
  draw_circle_helper(x+r    , y+r    , r, 1, color);
  draw_circle_helper(x+w-r-1, y+r    , r, 2, color);
  draw_circle_helper(x+w-r-1, y+h-r-1, r, 4, color);
  draw_circle_helper(x+r    , y+h-r-1, r, 8, color);
}

void fill_screen(uint16_t color) {
    fill_rect(0, 0,  _width, _height, color);
}

void set_cursor(int16_t x, int16_t y) {
  cursor_x = x;
  cursor_y = y;
}

void writech(uint8_t c) {
    if(c == '\n') {
      cursor_x  = 0;
      cursor_y += (int16_t)textsize *
                  gfxFont.yAdvance;
    } 
    else if (c != '\r') {
        uint8_t first = gfxFont.first;
        if ((c >= first) && (c <= gfxFont.last)) {
            uint8_t   c2    = c - gfxFont.first;
            GFXglyph *glyph = &(gfxFont.glyph)[c2];
            uint8_t   w     = glyph->width,
                      h     = glyph->height;
            if ((w > 0) && (h > 0)) { // Is there an associated bitmap?
                int16_t xo = glyph->xOffset; // sic
                if (text_wrap && ((cursor_x + textsize * (xo + w)) >= _width)) {
                    // Drawing character would go off right edge; wrap to new line
                    cursor_x  = 0;
                    cursor_y += (int16_t)textsize *
                                gfxFont.yAdvance;
                }
                draw_char(cursor_x, cursor_y, c, textcolor, textsize);
            }
            cursor_x += glyph->xAdvance * (int16_t)textsize;
        }
    }
}

// Draw a character
void draw_char(int16_t x, int16_t y, unsigned char c, uint16_t color, uint8_t size) {
    // Character is assumed previously filtered by write() to eliminate
    // newlines, returns, non-printable characters, etc.  Calling drawChar()
    // directly with 'bad' characters of font may cause mayhem!

    c -= gfxFont.first;
    GFXglyph *glyph  = &(gfxFont.glyph)[c];
    uint8_t  *bitmap = gfxFont.bitmap;

    uint16_t bo = glyph->bitmapOffset;
    uint8_t  w  = glyph->width,
             h  = glyph->height,
             xa = glyph->xAdvance;
    int8_t   xo = glyph->xOffset,
             yo = glyph->yOffset;
    uint8_t  xx, yy, bits, bitz = 0;
    int16_t  xo16, yo16;

    if(size > 1) {
      xo16 = xo;
      yo16 = yo;
    }
    for(yy = 0; yy < h; yy++) {
        for(xx = 0; xx < w; xx++) {
            if(!(bitz++ & 7)) {
                bits = bitmap[bo++];
            }
            if(bits & 0x80) {
                if(size == 1) {
                    draw_pixel(x+xo+xx, y+yo+yy, color);
                } else {
                    fill_rect(x+(xo16+xx)*size, y+(yo16+yy)*size, size, size, color);
                }
            }
            bits <<= 1;
        }
    }

  } // End classic vs custom font

// Draw a 1-bit image (bitmap) at the specified (x,y) position from the
// provided bitmap buffer (must be PROGMEM memory) using the specified
// foreground color (unset bits are transparent).
void draw_bitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color) {

  int16_t i, j, byteWidth = (w + 7) / 8;
  uint8_t byte;

  for(j = 0; j < h; j++) {
    for(i = 0; i < w; i++) {
      if(i & 7) byte <<= 1;
      else      byte   = *(bitmap + j * byteWidth + i / 8); 
      if(!(byte & 0x80)) draw_pixel(x+i, y+j, color);
    }
  }
}

// fill a rectangle
void fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {

    // rudimentary clipping (drawChar w/big text requires this)
    if((x >= _width) || (y >= _height)) return;
    if((x + w - 1) >= _width)  w = _width  - x;
    if((y + h - 1) >= _height) h = _height - y;

    set_window(x, y, x+w-1, y+h-1);

    uint8_t hi = color >> 8, lo = color;

    TFT_RS = 1;
    TFT_CS = 0;
    for(y = h; y > 0; y--) {
        for(x = w; x > 0; x--) {
            spi_write(hi);
            spi_write(lo);
        }
    }
    
    TFT_CS = 1;
}

// Pass 8-bit (each) R,G,B, get back 16-bit packed color
uint16_t color_565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_RGB 0x00
#define MADCTL_BGR 0x08
#define MADCTL_MH  0x04

void set_rotation(uint8_t m) {
    write_command(ST7735_MADCTL);
    rotation = m % 4; // can't be higher than 3
    switch (rotation) {
        case 0:
            write_data(MADCTL_MX | MADCTL_MY | MADCTL_BGR);
            _width  = ST7735_TFTWIDTH;
            _height = ST7735_TFTHEIGHT;
            break;
        case 1:
            write_data(MADCTL_MY | MADCTL_MV | MADCTL_BGR);
            _width = ST7735_TFTHEIGHT;
            _height = ST7735_TFTWIDTH;
            break;
        case 2:
            write_data(MADCTL_BGR);
            _width  = ST7735_TFTWIDTH;
            _height = ST7735_TFTHEIGHT;
            break;
        case 3:
            write_data(MADCTL_MX | MADCTL_MV | MADCTL_BGR);
            _width = ST7735_TFTHEIGHT;
            _height = ST7735_TFTWIDTH;
            break;
    }
}

void set_text_color(uint16_t c) {
  textcolor = c;
}

void set_text_size(uint8_t s) {
  textsize = (s > 0) ? s : 1;
}
