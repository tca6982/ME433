/* 
 * File:   main.c
 * Author: tmchiang
 *
 * Created on May 15, 2022, 10:15 PM
 */

#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include<stdio.h>
#include "ssd1306.h"
#include "i2c_master_noint.h"
#include "font.h"

// DEVCFG0
#pragma config DEBUG = OFF // disable debugging
#pragma config JTAGEN = OFF // disable jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // disable flash write protect
#pragma config BWP = OFF // disable boot write protect
#pragma config CP = OFF // disable code protect

// DEVCFG1
#pragma config FNOSC = FRCPLL // use fast frc oscillator with pll
#pragma config FSOSCEN = OFF // disable secondary oscillator
#pragma config IESO = OFF // disable switching clocks
#pragma config POSCMOD = OFF // primary osc disabled
#pragma config OSCIOFNC = OFF // disable clock output
#pragma config FPBDIV = DIV_1 // divide sysclk freq by 1 for peripheral bus clock
#pragma config FCKSM = CSDCMD // disable clock switch and FSCM
#pragma config WDTPS = PS1048576 // use largest wdt value
#pragma config WINDIS = OFF // use non-window mode wdt
#pragma config FWDTEN = OFF // wdt disabled
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the sysclk clock to 48MHz from the 8MHz fast rc internal oscillator
#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz
#pragma config FPLLMUL = MUL_24 // multiply clock after FPLLIDIV
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get 48MHz

// DEVCFG3
#pragma config USERID = 0 // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
#pragma config IOL1WAY = OFF // allow multiple reconfigurations

void drawChar(unsigned char x, unsigned char y, int row);
void drawString(unsigned char x, unsigned char y, int *rows, int size);
int *num2str(int num);

int main() {

    __builtin_disable_interrupts(); // disable interrupts while initializing things

    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;

    // do your TRIS and LAT commands here
    TRISBbits.TRISB4 = 1;
    TRISAbits.TRISA4 = 0;
    LATAbits.LATA4 = 0;
    
    __builtin_enable_interrupts();
    
    i2c_master_setup();
    ssd1306_setup();
    
    int str1[5] = {45, 37, 20, 19, 19};
    drawString(10, 5, str1, 5);
    int str2[11] = {52, 14, 45, 14, 0, 35, 40, 41, 33, 46, 39};
    drawString(50, 5, str2, 11);
    int str3[4] = {73, 0, 29, 0};
    drawString(60, 15, str3, 4);
    
    int count = 0;
    int *time;
    _CP0_SET_COUNT(0);
    while (1) {
        time = num2str(count); 
        drawString(85, 15, time, 5);
        count++;
        if (count > 99999) {
            count = 0;
        }
        ssd1306_update();
        
        if (_CP0_GET_COUNT()%48000000 < 48000000/2){
            LATAbits.LATA4 = 0;
        }
        else {
            LATAbits.LATA4 = 1;
        }
    }
}

void drawChar(unsigned char x, unsigned char y, int row){
    int i, j;
    unsigned char bit_map;
    for(i=0;i<5;i++){
        bit_map = ASCII[row][i];
        for(j=0;j<8;j++){
            ssd1306_drawPixel(x+i, y+j, (bit_map>>j)&1);
        }
    }
    //ssd1306_update();
}

void drawString(unsigned char x, unsigned char y, int *rows, int size){
    int i;
    for (i=0;i<size;i++){
        drawChar(x+5*i, y, rows[i]);
    }
}

int *num2str(int num){
    static int str[5] = {0, 0, 0, 0, 0};
    for (int i=0;i<5;i++){
        str[4-i] = num%10 + 16;
        num = num/10;
        if (num == 0) {
            return str;
        }
    }
}