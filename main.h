#ifndef MAIN_H
#define MAIN_H

#include <xc.h>

#define LSB_PORT	PORTA
#define LSB_PORT_LAT	LATA
#define LSB_PORT_DIR	TRISA

#define MSB_PORT	PORTC
#define MSB_PORT_LAT	LATC
#define	MSB_PORT_DIR	TRISC

// Octal latch
#define ALE     PORTBbits.RB0
#define ALE_LAT LATBbits.LATB0
#define ALE_DIR TRISBbits.RB0

// RAM
#define IOM      PORTDbits.RD3
#define IOM_LAT  LATDbits.LATD3
#define IOM_DIR  TRISDbits.RD3

#define RD      PORTBbits.RB1
#define RD_LAT  LATBbits.LATB1
#define RD_DIR  TRISBbits.RB1

#define WR      PORTBbits.RB2
#define WR_LAT  LATBbits.LATB2
#define WR_DIR  TRISBbits.RB2

#define MEM_SIZE    2048
#define MEM_MAX     MEM_SIZE - 1
#define MEM_MIN     0x50

const unsigned char logo[] = {
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xef,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xe7,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xe7,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xcf,0xff,0xff,0xff,0xff,0xc0,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xcf,0xff,0xff,0xff,0xfe,0x6,0x3f,0xff,0xe0,0xf,
    0xfe,0x0,0x1f,0xff,0xcf,0xff,0xff,0xff,0xf8,0x3,0x9f,0x80,0xf,0xff,
    0xf0,0x1f,0x81,0xff,0xcf,0xff,0xff,0xff,0xf1,0xff,0x9f,0xcf,0xff,0xff,
    0xfe,0x7f,0xf8,0x7f,0x9f,0xff,0xfe,0x3f,0xe7,0xff,0x9f,0x9f,0xff,0xff,
    0xfe,0x7f,0xfc,0x7f,0x9f,0x8f,0xfc,0xf,0xcf,0xff,0x3f,0x3f,0xff,0xff,
    0xfc,0x7f,0xfe,0x7f,0x9e,0xf,0xf8,0xcf,0x8f,0xfc,0x7e,0x3f,0xff,0xff,
    0xfc,0xff,0xfc,0xff,0x3c,0xcf,0xf1,0xc7,0xcf,0xf9,0xfc,0x7f,0xff,0xff,
    0xfc,0xff,0xe1,0xff,0x39,0xcf,0xe7,0xe7,0xcf,0xe3,0xf8,0x1,0xff,0xff,
    0xf8,0x1c,0xf,0xff,0x73,0xcf,0xc7,0xe7,0xef,0x9f,0xf0,0x7c,0x3f,0xff,
    0xf8,0x1,0xff,0xfe,0x67,0xdf,0xcf,0xef,0xe6,0x3f,0xff,0xff,0x1f,0xff,
    0xf9,0x8f,0xff,0xfe,0xcf,0x9f,0x9f,0xcf,0xf0,0xff,0xff,0xff,0x9f,0xff,
    0xf3,0xc7,0xff,0xfc,0x8f,0x9f,0xbf,0xcf,0xe0,0xff,0xff,0xff,0x9f,0xff,
    0xf3,0xe3,0xff,0xfc,0x1f,0x3f,0x3f,0x9f,0xce,0x7f,0xff,0xff,0x3f,0xff,
    0xf3,0xf3,0xff,0xf8,0x3f,0x3e,0x7f,0xbf,0x1f,0x3f,0x7f,0xfe,0x7f,0xff,
    0xe7,0xf1,0xff,0xf8,0x7f,0x3e,0x7f,0x7e,0x7f,0xbf,0x7f,0xfc,0xff,0xff,
    0xe7,0xf8,0xff,0xf8,0xfe,0x7e,0xfc,0xfc,0xff,0xde,0x7f,0xf9,0xff,0xff,
    0xe7,0xfc,0x3f,0x11,0xfe,0x7c,0xf9,0xfc,0xff,0x9e,0x3f,0xe3,0xff,0xff,
    0xcf,0xff,0x80,0xf3,0xfe,0x7c,0xe3,0xfc,0x7f,0x3f,0x86,0xf,0xff,0xff,
    0xff,0xff,0xff,0xff,0xfe,0x7c,0xf,0xff,0x0,0x7f,0xf0,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff
};

#endif