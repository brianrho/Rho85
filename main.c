/* File:      main.c
 * Author:    Brian Ejike
 * Created:   Tue Mar 15 2016
 * Processor: PIC18F4520
 * Compiler:  MPLAB XC8
 */

#include <xc.h>
#include "my_uart.h"
#include "main.h"
#include "ram_mem.h"
#include "tft_screen.h"
#include "utils.h"
#include "mpu.h"
#include "keypad.h"
#include <string.h>

// CONFIG1H
#pragma config OSC = INTIO67    // Oscillator Selection bits (Internal oscillator block, port function on RA6 and RA7)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor disabled)
#pragma config IESO = OFF       // Internal/External Oscillator Switchover bit (Oscillator Switchover mode disabled)

// CONFIG2L
#pragma config PWRT = ON        // Power-up Timer Enable bit (PWRT enabled)
#pragma config BOREN = OFF      // Brown-out Reset Enable bits (Brown-out Reset disabled in hardware and software)
#pragma config BORV = 3         // Brown Out Reset Voltage bits (Minimum setting)

// CONFIG2H
#pragma config WDT = OFF        // Watchdog Timer Enable bit (WDT disabled (control is placed on the SWDTEN bit))
#pragma config WDTPS = 32768    // Watchdog Timer Postscale Select bits (1:32768)

// CONFIG3H
#pragma config CCP2MX = PORTC   // CCP2 MUX bit (CCP2 input/output is multiplexed with RC1)
#pragma config PBADEN = OFF     // PORTB A/D Enable bit (PORTB<4:0> pins are configured as digital I/O on Reset)
#pragma config LPT1OSC = OFF    // Low-Power Timer1 Oscillator Enable bit (Timer1 configured for higher power operation)
#pragma config MCLRE = ON       // MCLR Pin Enable bit (MCLR pin enabled; RE3 input pin disabled)

// CONFIG4L
#pragma config STVREN = ON      // Stack Full/Underflow Reset Enable bit (Stack full/underflow will cause Reset)
#pragma config LVP = OFF        // Single-Supply ICSP Enable bit (Single-Supply ICSP disabled)
#pragma config XINST = OFF      // Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode disabled (Legacy mode))

// CONFIG5L
#pragma config CP0 = OFF        // Code Protection bit (Block 0 (000800-001FFFh) not code-protected)
#pragma config CP1 = OFF        // Code Protection bit (Block 1 (002000-003FFFh) not code-protected)
#pragma config CP2 = OFF        // Code Protection bit (Block 2 (004000-005FFFh) not code-protected)
#pragma config CP3 = OFF        // Code Protection bit (Block 3 (006000-007FFFh) not code-protected)

// CONFIG5H
#pragma config CPB = OFF        // Boot Block Code Protection bit (Boot block (000000-0007FFh) not code-protected)
#pragma config CPD = OFF        // Data EEPROM Code Protection bit (Data EEPROM not code-protected)

// CONFIG6L
#pragma config WRT0 = OFF       // Write Protection bit (Block 0 (000800-001FFFh) not write-protected)
#pragma config WRT1 = OFF       // Write Protection bit (Block 1 (002000-003FFFh) not write-protected)
#pragma config WRT2 = OFF       // Write Protection bit (Block 2 (004000-005FFFh) not write-protected)
#pragma config WRT3 = OFF       // Write Protection bit (Block 3 (006000-007FFFh) not write-protected)

// CONFIG6H
#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration registers (300000-3000FFh) not write-protected)
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot block (000000-0007FFh) not write-protected)
#pragma config WRTD = OFF       // Data EEPROM Write Protection bit (Data EEPROM not write-protected)

// CONFIG7L
#pragma config EBTR0 = OFF      // Table Read Protection bit (Block 0 (000800-001FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR1 = OFF      // Table Read Protection bit (Block 1 (002000-003FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR2 = OFF      // Table Read Protection bit (Block 2 (004000-005FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR3 = OFF      // Table Read Protection bit (Block 3 (006000-007FFFh) not protected from table reads executed in other blocks)

// CONFIG7H
#pragma config EBTRB = OFF      // Boot Block Table Read Protection bit (Boot block (000000-0007FFh) not protected from table reads executed in other blocks)

#define _XTAL_FREQ  32000000

#define INVALID_ADDRESS 1
#define INVALID_OPERATION 2

// prototypes for the state functions
void entering_idle();
void idle_state();
void subst_mem();
void enter_data();
void goto_addr();
void waiting_mpu();
void exam_reg();
void display_next_reg(char);
void raise_err(char);
void read_cpu_regs();

// states
typedef enum {
    ENTERING_IDLE,
    IDLE,
    SUBST_MEM,
    ENTER_DATA,
    GOTO_ADDR,
    WAITING_MPU,
    EXAM_REG,
} State_Type;

State_Type curr_state = ENTERING_IDLE;

// array of state functions
void (*state_table[])() = {entering_idle, idle_state, subst_mem, enter_data, goto_addr, waiting_mpu, exam_reg};
uint8_t REG8[] = {0, 0, 0, 0, 0, 0, 0, 0};
uint16_t REG16[] = {0, 0};

// 8085 register names
const char * reg_names[] = {"A", "B", "C", "D", "E", "H", "L", "F", "PC", "SP"};

static uint16_t curr_addr;  // stores the current memory address between function calls
static uint8_t dat = 0;     // stores the data in the LCD entry box between function calls

// initial stack pointer value
volatile uint16_t STACK_PTR = 0x7ff;

void init_system(){
    init_mpu_interface();   // reset the 8085 and initialize the status pins
    
    uart_init();        // init uart; not really needed in the final device, used for debugging
    __delay_ms(20);     // necessary delay after init uart to not get gibberish
    ADCON1 |= 0xf;      // make PORTA pins digital
    CMCON |= 0x7;       // make sure comparators are off
    mem_config(RW);     // configure port pins for R/W operations
    
    init_keypad();      // init mcp23s17 expander
    init_screen();      // setup the screen
    set_rotation(1);
    
    println("System ready.", uart_write);
    fill_screen(ST7735_WHITE);
    draw_bitmap(24, 50, logo, 112, 29, ST7735_RED);
    delay_ms(2000);
}

void main(void) {
    OSCCON |= 0x70; // select internal clock
    OSCTUNEbits.PLLEN = 1; // Enable x4 PLL; Fosc = 32 MHz
    
    init_system();
    while (1){      // begin state cycle
        state_table[curr_state]();
    }
}

// transition function to set up the screen before entering the IDLE state
void entering_idle(){       
    fill_screen(ST7735_BLACK);
    set_cursor(45, 64);
    set_text_color(ST7735_GREEN);
    set_text_size(1);
    println("READY", writech);
    curr_state = IDLE;
}

void idle_state(){
    char c = scan_keypad();     // grab a key, if any
    static char shift = 0;      // shift is initially off; retains state between function calls
    
    switch (c){
        case 'S':               // if shift was pressed, use a tiny green rectangle to indicate
            if (shift){
                fill_rect(1, 1, 3, 3, ST7735_BLACK);
                shift = 0;
            }
            else{
                fill_rect(1, 1, 3, 3, ST7735_GREEN);
                shift = 1;
            }
            break;
        case 'M':
            if (shift){
                curr_state = GOTO_ADDR;
                shift = 0;
            }
            else{                
                curr_state = SUBST_MEM;
            }
            fill_screen(ST7735_BLACK);          // same transition for SUBST_MEM and GOTO_ADDR
            draw_round_rect(40, 60, 80, 45, 5, ST7735_RED);
            set_cursor(32, 42);
            set_text_color(ST7735_RED);
            set_text_size(1);
            print("ADDRESS", writech);
            set_cursor(52, 90);
            set_text_color(ST7735_WHITE);
            printnum(0, H_ADDR, writech);
            break;
        case 'R': 
            if (shift){         // set up the screen; render the contents of the first register (A)
                fill_screen(ST7735_BLACK);
                draw_round_rect(40, 60, 80, 45, 5, ST7735_RED);
                set_text_color(ST7735_RED);
                set_text_size(1);
                set_cursor(75, 42);
                print(reg_names[0], writech);
                set_text_size(2);
                set_cursor(52, 96);
                set_text_color(ST7735_WHITE);
                printnum(REG8[0], H_DATA, writech);
                
                shift = 0;
                curr_state = EXAM_REG;
            }
            else {          // clear the arrays, reset the 8085, and IDLE away
                memset(REG8, 0, sizeof(REG8)); memset(REG16, 0, sizeof(REG16));
                reset_mpu();
                curr_state = ENTERING_IDLE;
            }
        default:
            break;
    }
}

void subst_mem(){
    static uint16_t memloc = 0;
    char c = scan_keypad();
    
    if (c){
        if (c >= '0' && c <= '9'){
            set_cursor(52, 90);
            set_text_color(ST7735_BLACK);
            printnum(memloc, H_ADDR, writech);
            memloc = (memloc << 4) | (c - '0');
            set_cursor(52, 90);
            set_text_color(ST7735_WHITE);
            printnum(memloc, H_ADDR, writech);
        }
        else if (c >= 'A' && c <= 'F'){
            set_cursor(52, 90);
            set_text_color(ST7735_BLACK);
            printnum(memloc, H_ADDR, writech);
            memloc = (memloc << 4) | (c - 'A' + 10);
            set_cursor(52, 90);
            set_text_color(ST7735_WHITE);
            printnum(memloc, H_ADDR, writech);
        }
        else if (c == 'N'){
            if (memloc >= MEM_MIN && memloc <= MEM_MAX){
                curr_addr = memloc;
                mem_config(RW);
                memloc = 0;
                dat = read_mem(curr_addr);
                
                fill_screen(ST7735_BLACK);
                draw_round_rect(40, 60, 80, 45, 5, ST7735_RED);
                set_text_color(ST7735_RED);
                set_text_size(1);
                set_cursor(52, 42);
                printnum(curr_addr, H_ADDR, writech);
                set_text_size(2);
                set_cursor(52, 96);
                set_text_color(ST7735_WHITE);
                printnum(dat, H_DATA, writech);
                
                curr_state = ENTER_DATA;
            }
            else {
                raise_err(INVALID_ADDRESS);
                memloc = 0;
                curr_state = ENTERING_IDLE;
            }
        }
        else if (c == 'R'){
            reset_mpu();
            memset(REG8, 0, sizeof(REG8)); memset(REG16, 0, sizeof(REG16));
            memloc = 0;
            curr_state = ENTERING_IDLE;
        }
    }
}


void enter_data(){
    static char shift = 0;
    char c = scan_keypad();
    
    if (c){
        if (c >= '0' && c <= '9'){
            set_cursor(52, 96);
            set_text_color(ST7735_BLACK);
            printnum(dat, H_DATA, writech);
            dat = (dat << 4) | (c - '0');
            set_cursor(52, 96);
            set_text_color(ST7735_WHITE);
            printnum(dat, H_DATA, writech);
        }
        else if (c >= 'A' && c <= 'F'){
            set_cursor(52, 96);
            set_text_color(ST7735_BLACK);
            printnum(dat, H_DATA, writech);
            dat = (dat << 4) | (c - 'A' + 10);
            set_cursor(52, 96);
            set_text_color(ST7735_WHITE);
            printnum(dat, H_DATA, writech);
        }
        else if (c == 'S'){
            if (shift){
                fill_rect(1, 1, 3, 3, ST7735_BLACK);
                shift = 0;
            }
            else{
                fill_rect(1, 1, 3, 3, ST7735_GREEN);
                shift = 1;
            }
        }
        else if (c == 'N'){  // NEXT
            write_mem(curr_addr, dat);
            
            if (shift){  // EXEC
                curr_state = ENTERING_IDLE;
                shift = 0;
                dat = 0;
            }
            else {
                set_text_color(ST7735_BLACK);
                set_text_size(1);
                set_cursor(52, 42);
                printnum(curr_addr, H_ADDR, writech);
                curr_addr++;
                if (curr_addr <= MEM_MIN || curr_addr >= MEM_MAX){
                    raise_err(INVALID_ADDRESS);
                    curr_state = ENTERING_IDLE;
                    dat = 0;
                    shift = 0;
                    return;
                }
                
                set_text_color(ST7735_RED);
                set_cursor(52, 42);
                printnum(curr_addr, H_ADDR, writech);
                
                set_text_size(2);
                set_cursor(52, 96);
                set_text_color(ST7735_BLACK);
                printnum(dat, H_DATA, writech);
                dat = read_mem(curr_addr);
                set_cursor(52, 96);
                set_text_color(ST7735_WHITE);
                printnum(dat, H_DATA, writech);
            }
        }
        else if (c == 'R'){
            if (!shift){
                reset_mpu();
                dat = 0;
                memset(REG8, 0, sizeof(REG8)); memset(REG16, 0, sizeof(REG16));
                curr_state = ENTERING_IDLE;
            }
        }
    }
}
        
void goto_addr(){
    static uint16_t memloc = 0;
    char c = scan_keypad();
    static char shift = 0;
    
    if (c){
        if (c >= '0' && c <= '9'){
            set_cursor(52, 90);
            set_text_color(ST7735_BLACK);
            printnum(memloc, H_ADDR, writech);
            memloc = (memloc << 4) | (c - '0');
            set_cursor(52, 90);
            set_text_color(ST7735_WHITE);
            printnum(memloc, H_ADDR, writech);
        }
        else if (c >= 'A' && c <= 'F'){
            set_cursor(52, 90);
            set_text_color(ST7735_BLACK);
            printnum(memloc, H_ADDR, writech);
            memloc = (memloc << 4) | (c - 'A' + 10);
            set_cursor(52, 90);
            set_text_color(ST7735_WHITE);
            printnum(memloc, H_ADDR, writech);
        }
        else if (c == 'S'){
            if (shift){
                fill_rect(1, 1, 3, 3, ST7735_BLACK);
                shift = 0;
            }
            else{
                fill_rect(1, 1, 3, 3, ST7735_GREEN);
                shift = 1;
            }
        }
        else if (c == 'N'){
            if (shift){
                if (memloc >= MEM_MIN && memloc <= MEM_MAX){
                    curr_addr = memloc;
                    fill_screen(ST7735_BLACK);
                    set_cursor(15, 60);
                    set_text_color(ST7735_GREEN);
                    set_text_size(1);
                    print("Running...", writech);
                    start_mpu(curr_addr); // load JMP instr, tristate and release from reset
                    memloc = 0;
                    shift = 0;
                    curr_state = WAITING_MPU;
                    
                }
                else {
                    raise_err(INVALID_ADDRESS);
                    memloc = 0;
                    curr_state = ENTERING_IDLE;
                }
            }
        }
        else if (c == 'R'){
            if (!shift){
                reset_mpu();
                memset(REG8, 0, sizeof(REG8)); memset(REG16, 0, sizeof(REG16));
                curr_state = ENTERING_IDLE;
                memloc = 0;
            }
        }
    }
}

void waiting_mpu(){
    char c = scan_keypad();
    if (c == 'R'){
        reset_mpu();
        memset(REG8, 0, sizeof(REG8)); memset(REG16, 0, sizeof(REG16));
        curr_state = ENTERING_IDLE;
    }
    else if ((!STAT0) && (!STAT1)){
        mem_config(RW);
        read_cpu_regs();
        set_text_color(ST7735_BLACK);
        set_cursor(15, 60); print("Running...", writech);
        set_text_color(ST7735_WHITE);
        set_cursor(40, 60); print("Done.", writech);
        curr_state = ENTERING_IDLE;
    }
}

void display_next_reg(char index){
    fill_screen(ST7735_BLACK);
    set_text_color(ST7735_RED);
    set_text_size(1);
    if (index < 7){
        draw_round_rect(40, 60, 80, 45, 5, ST7735_RED);
        set_cursor(75, 42);
        print(reg_names[index], writech);
        set_text_size(2);
        set_cursor(52, 96);
        set_text_color(ST7735_WHITE);
        printnum(REG8[index], H_DATA, writech);
    }
    else if (index == 7){
        set_cursor(15, 42); print("S", writech);
        set_cursor(40, 42); print("Z", writech);
        set_cursor(65, 42); print("AC", writech);
        set_cursor(105, 42); print("P", writech);
        set_cursor(130, 42); print("C", writech);
        set_text_color(ST7735_WHITE); 
        set_cursor(15, 96); printnum((REG8[index] & (1 << 7)) ? 1 : 0, DEC, writech);
        set_cursor(40, 96); printnum((REG8[index] & (1 << 6)) ? 1 : 0, DEC, writech);
        set_cursor(72, 96); printnum((REG8[index] & (1 << 4)) ? 1 : 0, DEC, writech);
        set_cursor(105, 96); printnum((REG8[index] & (1 << 2)) ? 1 : 0, DEC, writech);
        set_cursor(130, 96); printnum((REG8[index] & 1) ? 1 : 0, DEC, writech);
    }
    else if (index > 7){
        draw_round_rect(40, 60, 80, 45, 5, ST7735_RED);
        set_cursor(72, 42);
        print(reg_names[index], writech);
        set_cursor(52, 90);
        set_text_color(ST7735_WHITE);
        printnum(REG16[index - 8], H_ADDR, writech);
    }
}

void exam_reg(){
    char c = scan_keypad();
    static char index = 1;
    static char shift = 0;
    
    if (c == 'S'){
        if (shift){
            fill_rect(1, 1, 3, 3, ST7735_BLACK);
            shift = 0;
        }
        else{
            fill_rect(1, 1, 3, 3, ST7735_GREEN);
            shift = 1;
        }
    }
    else if (c == 'N'){
        if (shift){
            shift = 0;
            index = 1;
            curr_state = ENTERING_IDLE;
        }
        else {
            if (index == NUM_REGS){
                index = 1;
                curr_state = ENTERING_IDLE;
            }
            else {
                display_next_reg(index);
                index++;
            }
        }
    }
    else if (c == 'R'){
        reset_mpu();
        index = 1; shift = 0;
        memset(REG8, 0, sizeof(REG8)); memset(REG16, 0, sizeof(REG16));
        curr_state = ENTERING_IDLE;
    }
}

void read_cpu_regs(){ 
    REG8[0] = read_mem(0x36); REG8[1] = read_mem(0x35);  // A, B
    REG8[2] = read_mem(0x34); REG8[3] = read_mem(0x33);  // C, D
    REG8[4] = read_mem(0x32); REG8[5] = read_mem(0x31);  // E, H
    REG8[6] = read_mem(0x30);  // L
    STACK_PTR = 4 + (((uint16_t)(read_mem(0x38)) << 8) | read_mem(0x37));  // +4 is to offset for POP and RST1
    REG16[0] = ((uint16_t)(read_mem(STACK_PTR - 1)) << 8) | read_mem(STACK_PTR - 2);
    REG16[1] = STACK_PTR;
    REG8[7] = read_mem(STACK_PTR - 4);  // flags
}
    
void raise_err(char err_type){
    fill_screen(ST7735_BLACK);
    set_text_color(ST7735_RED);
    set_cursor(30, 40); print("Invalid", writech);
    set_cursor(25, 60);
    switch (err_type){
        case INVALID_ADDRESS:
            print("address!", writech);
            break;
        case INVALID_OPERATION:
            print("operation!", writech);
            break;
        default:
            break;
    }
    delay_ms(1000);
}
/*
void interrupt isr(void){
    if (INT0IE && INT0IF){
        uint16_t temp = ((uint16_t)(MSB_PORT & 0x7) << 8) | LSB_PORT;
        PROG_COUNT++;
        if ((temp == POP_ADDR) && ((PORTD & MPU_STATUS_MASK) == OPCODE_FETCH)){
            other++;
            while (!ALE);  // should include a timeout
            while (ALE);
            temp = ((uint16_t)(MSB_PORT & 0x7) << 8) | LSB_PORT;
            STACK_PTR = temp + 4;   // offset the RST1 jump
        }
        INT0IF = 0;
    }
}*/