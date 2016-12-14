#ifndef KEYPAD_H
#define KEYPAD_H

#include <xc.h>

#define _XTAL_FREQ  32000000

#define MCP_MOSI_DIR    TRISDbits.RD6
#define MCP_MISO_DIR    TRISDbits.RD7
#define MCP_SCK_DIR     TRISDbits.RD5
#define MCP_CS_DIR      TRISDbits.RD4

#define MCP_MOSI        LATDbits.LATD6
#define MCP_MISO        PORTDbits.RD7
#define MCP_SCK         LATDbits.LATD5
#define MCP_CS          LATDbits.LATD4

enum {
    IODIRA,     IODIRB,
    IPOLA,      IPOLB,
    GPINTENA,   GPINTENB,
    DEFVALA,    DEFVALB,
    INTCONA,    INTCONB,
    IOCONA,     IOCONB,
    GPPUA,      GPPUB,
    INTFA,      INTFB,
    INTCAPA,    INTCAPB,
    GPIOA,      GPIOB,
    OLATA,      OLATB
};

unsigned char regs[22];

void init_keypad();
char scan_keypad();
void read_all();

#endif