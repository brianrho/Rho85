#ifndef MY_UART_H
#define MY_UART_H

void uart_init(void);
void uart_write(char c);
void uart_print(const char * str);
void uart_println(const char *str);
void uart_printnum(unsigned int num, char base);

#endif