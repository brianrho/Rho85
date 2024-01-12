/* 
 * File:   mpu.h
 * Author: Brian Rho
 *
 * Created on 01 June 2016, 00:08
 */

#include "ram_mem.h"

// PIC-8085 control pins
#define RESET_IN_DIR    TRISCbits.RC5
#define STAT0_DIR   TRISDbits.RD1
#define STAT1_DIR   TRISDbits.RD2
#define RST75_DIR   TRISCbits.RC7

#define RESET_IN    LATCbits.LATC5
#define STAT0   PORTDbits.RD2
#define STAT1   PORTDbits.RD3
#define RST75   LATCbits.LATC7

// some 8085 instructions
#define JUMP    0xC3

#define MOVB    0x78
#define MOVC    0x79
#define MOVD    0x7A
#define MOVE    0x7B
#define MOVH    0x7C
#define MOVL    0x7D

#define STA     0x32
#define NOOP     0x00
#define SPHL    0xF9
#define MVIH    0x26
#define MVIL    0x2E

#define NUM_REGS    10

void init_mpu_interface(void);
void reset_mpu(void);
void halt_mpu(void);
void start_mpu(unsigned int addr);
