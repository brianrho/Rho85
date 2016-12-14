#ifndef RAM_MEM_H
#define RAM_MEM_H

#include "main.h"

#define _XTAL_FREQ 32000000

#define RW	1
#define TRISTATE 2

void mem_config(char state);
unsigned char read_mem(unsigned int addr);
void write_mem(unsigned int addr, unsigned char dat);
void write_mem_cont(unsigned int start, unsigned char * data, unsigned char len);
void read_mem_cont(unsigned int start, unsigned char buf[], unsigned char len);

#endif