#include "ram_mem.h"

void mem_config(char state){
    if (state == TRISTATE){
      LSB_PORT_DIR = 0xff;  // AD0-AD7
      MSB_PORT_DIR |= 0x7;  // A8-A10
      IOM_DIR = 1;
      RD_DIR = 1;
      WR_DIR = 1;
      ALE_DIR = 1;
    }
    else if (state == RW){
      ALE_DIR = 0; ALE_LAT = 1;
      IOM_DIR = 0; IOM_LAT = 1;
      RD_DIR = 0; RD_LAT = 1;
      WR_DIR = 0; WR_LAT = 1;
    }
 }


unsigned char read_mem(unsigned int addr){
    unsigned char dat;
    LSB_PORT_DIR = 0x00;
    MSB_PORT_DIR &= 0xf8;
    ALE_LAT = 1;
    LSB_PORT_LAT = (unsigned char)(addr & 0xff);
    __delay_us(10);
    ALE_LAT = 0;            // latch addr to bus
    MSB_PORT_LAT &= 0xf8;
    MSB_PORT_LAT |= ((unsigned char)(addr >> 8)) & 0x7;
    __delay_us(10);
    LSB_PORT_DIR = 0xff;
    __delay_us(10);
    IOM_LAT = 0; __delay_us(10); RD_LAT = 0;    // enable RAM, tristate 595 LSB, enable RAM output
    __delay_us(10);
    dat = LSB_PORT;
    RD_LAT = 1; IOM_LAT = 1;         // tristate RAM
    return dat;
}

void write_mem(unsigned int addr, unsigned char val){
    LSB_PORT_DIR = 0x00;
    MSB_PORT_DIR &= 0xf8;
   
    ALE_LAT = 1;
    LSB_PORT_LAT = (unsigned char)(addr & 0xff);
    __delay_us(10);
    ALE_LAT = 0;            // latch addr to bus
    
    MSB_PORT_LAT &= 0xf8;
    MSB_PORT_LAT |= ((unsigned char)(addr >> 8)) & 0x7;
    __delay_us(10);
    LSB_PORT_LAT = val;
    
    IOM_LAT = 0; __delay_us(10); WR_LAT = 0;
    __delay_us(10);
    WR_LAT = 1; IOM_LAT = 1;
}

void write_mem_cont(unsigned int start, unsigned char * data, unsigned char len){
    unsigned int i;
    for (i = 0; i < len; i++){
        write_mem(start + i, data[i]);
    }
}

void read_mem_cont(unsigned int start, unsigned char buf[], unsigned char len){
    unsigned int i;
    for (i = 0; i < len; i++){
        buf[i] = read_mem(start + i);
    }
}