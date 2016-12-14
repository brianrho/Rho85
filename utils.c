#include "utils.h"
#include <xc.h>
#include <stdio.h>

#define _XTAL_FREQ 32000000

void print(const char *str, void (*func)(uint8_t c)){
    while (*str)
        (*func)(*str++);
}

void printnum(unsigned long num, char pad, void (*func)(uint8_t c)){
    char buf[10];
    if (pad == H_DATA)
        sprintf(buf, "%.2X", num);
    else if (pad == H_ADDR)
        sprintf(buf, "%.4X", num);
    else if (pad == DEC)
        sprintf(buf, "%d", num);
    print(buf, func);
}

void println(const char *str, void (*func)(uint8_t c)){
    print(str, func);
    print("\r\n", func);
}

void println_num(unsigned long num, char pad, void (*func)(uint8_t c)){
    printnum(num, pad, func);
    print("\r\n", func);
}

void delay_ms(uint32_t ms){
    while (ms--)
        __delay_ms(1);
}
