#include <xc.h>
#include "my_uart.h"
#include <stdlib.h>

void uart_init(void){
    TRISCbits.RC6 = 0;  // TX

    SPBRG = 51;
    TXSTA |= 0x20;  // Enable transmission
    RCSTA |= 0x80;  // Enable serial port
}

void uart_write(char c){
    while (!TXSTAbits.TRMT);
    TXREG = c;
}

void uart_print(const char * str){
    while (*str)
        uart_write(*str++);
}

void uart_println(const char *str){
    uart_print(str);
    uart_print("\r\n");
}

void uart_printnum(unsigned int num, char base){
    char buf[10];
    utoa(buf, num, base);
    uart_print(buf);
}
