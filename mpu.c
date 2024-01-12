#include "mpu.h"

void init_mpu_interface(void){
    reset_mpu();
    STAT0_DIR = 1;
    STAT1_DIR = 1;
}

void reset_mpu(void){
    RESET_IN_DIR = 0;
    RESET_IN = 0;
    __delay_ms(1);
}

void start_mpu(unsigned int addr){
    RESET_IN_DIR = 1;
    //__delay_ms(100);
}