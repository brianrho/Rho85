#include "keypad.h"

char keys[5][4] = {
  {'S','M','N','R'},
  {'1','2','3','F'},
  {'4','5','6','E'},
  {'7','8','9','D'},
  {'A','0','B','C'}
};

unsigned char spi_transfer(unsigned char data){
    unsigned char ret = 0;

    MCP_SCK = 0;
    for (unsigned char bits = 0x80; bits; bits >>= 1){
        MCP_MOSI = (data & bits) ? 1 : 0;
        MCP_SCK = 1;
        ret <<= 1;
        ret |= MCP_MISO;
        __delay_us(1);
        MCP_SCK = 0;
        __delay_us(1);
    }

    return ret;
}

void init_keypad(){
    MCP_MOSI_DIR = 0;
    MCP_MISO_DIR = 1;
    MCP_SCK_DIR = 0;
    MCP_CS_DIR = 0;
    
    MCP_CS = 1;
    
    regs[IODIRA] = 0xE0;
    regs[IODIRB] = 0xFF;
    regs[IPOLA] = 0x00;
    regs[IPOLB] = 0x0F;
    regs[GPINTENA] = 0x00;
    regs[GPINTENB] = 0x00;
    regs[DEFVALA] = 0x00;
    regs[DEFVALB] = 0x00;
    regs[INTCONA] = 0x00;
    regs[INTCONB] = 0x00;
    regs[IOCONA] = 0x18;
    regs[IOCONB] = 0x18;
    regs[GPPUA] = 0x00;
    regs[GPPUB] = 0x0F;
    regs[INTFA] = 0x00;
    regs[INTFB] = 0x00;
    regs[INTCAPA] = 0x00;
    regs[INTCAPB] = 0x00;
    regs[GPIOA] = 0x00;
    regs[GPIOB] = 0x00;
    regs[OLATA] = 0xFF;
    regs[OLATB] = 0x00;
    
    unsigned char cmd = 0b01000000;
    MCP_CS = 0;
    spi_transfer(cmd);
    spi_transfer(IOCONA);
    spi_transfer(0x18);
    MCP_CS = 1;
    
    MCP_CS = 0;
    spi_transfer(cmd);
    spi_transfer(0);
    for (char i = 0; i < 22; i++){
        spi_transfer(regs[i]);
    }
    MCP_CS = 1;
}

void write_register(unsigned char addr, unsigned char val){
    unsigned char cmd = 0b01000000;
    MCP_CS = 0;
    spi_transfer(cmd);
    spi_transfer(addr);
    regs[addr] = val;
    spi_transfer(regs[addr]);
    MCP_CS = 1;
}

void read_register(unsigned char addr){
    unsigned char cmd = 0b01000001;
    MCP_CS = 0;
    spi_transfer(cmd);
    spi_transfer(addr);
    regs[addr] = spi_transfer(0xFF);
    MCP_CS = 1;
}

void read_all(){
    unsigned char cmd = 0b01000001;
    MCP_CS = 0;
    spi_transfer(cmd);
    spi_transfer(0);
    for (char i = 0; i < 22; i++){
        regs[i] = spi_transfer(0xFF);
    }
    MCP_CS = 1;
}
unsigned char scan_keypad(){
    char temp = 0, col = 0;
    
    for (char row = 0; row < 5; row++){
        write_register(OLATA, ~(1 << row));
        read_register(GPIOB);
        if (regs[GPIOB] & 0xF){
            __delay_ms(5);
            read_register(GPIOB);
            if (regs[GPIOB] & 0xF){
                temp = regs[GPIOB] & 0xF;
                while (temp >>= 1)
                    col++;
                while (regs[GPIOB] & 0xF)
                    read_register(GPIOB);
                return keys[row][col];
            }
        }
    }
    
    return 0;
}