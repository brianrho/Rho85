#ifndef UTILS_H
#define UTILS_H

#define H_DATA  2
#define H_ADDR  4
#define DEC     10

#include <stdlib.h>
#include <stdint.h>

void print(const char *str, void (*func)(uint8_t c));
void printnum(unsigned long num, char pad, void (*func)(uint8_t c));
void println(const char *str, void (*func)(uint8_t c));
void println_num(unsigned long num, char pad, void (*func)(uint8_t c));
void delay_ms(uint32_t ms);

#endif