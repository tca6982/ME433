/* 
 * File:   main.c
 * Author: tmchiang
 *
 * Created on May 11, 2022, 12:58 PM
 */

#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include<stdio.h>
#include "i2c_master_noint.h"

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

unsigned char getMCP23008pin(unsigned char byte);
void setMCP23008pin(unsigned char byte, unsigned char val);

//int main(){
    //__builtin_disable_interrupts(); // disable interrupts while initializing things

    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    //__builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    //BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    //INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    //DDPCONbits.JTAGEN = 0;
    
    //unsigned char writePin;
    //i2c_master_setup();
    //if (getMCP23008pin(readPin)) {
    //    setMCP23008pin(writePin, 0);
    //} else {
    //    setMCP23008pin(writePin, 1);
    //}
    //writePin = 128;
    //setMCP23008pin(writePin, 1);
    //LATAbits.LATA4 = 1;
    //_CP0_SET_COUNT(0);
    //while (_CP0_GET_COUNT() < 48000000 / 2 / 10){
    //}
    //LATAbits.LATA4 = 0;
    //while (_CP0_GET_COUNT() < 48000000 / 2 / 10){
    //}

//}

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
    
    unsigned char writePin;
    unsigned char readPin;
    writePin = 0x0A;
    readPin = 0x09;
    i2c_master_setup();
    setMCP23008pin(0x00, 0b01111111);
    while (1) {
        if (getMCP23008pin(readPin)) {
            setMCP23008pin(writePin, 0b00000000);
        }else {
            setMCP23008pin(writePin, 0b10000000);
        }
        LATAbits.LATA4 = 1;
        _CP0_SET_COUNT(0);
        while (_CP0_GET_COUNT() < 48000000/2/10);
        LATAbits.LATA4 = 0;
        _CP0_SET_COUNT(0);
        while (_CP0_GET_COUNT() < 48000000/2/10);
    }
}

void i2c_master_setup(void) {
    // using a large BRG to see it on the nScope, make it smaller after verifying that code works
    // look up TPGD in the datasheet
    I2C1BRG = 1000; // I2CBRG = [1/(2*Fsck) - TPGD]*Pblck - 2 (TPGD is the Pulse Gobbler Delay)
    I2C1CONbits.ON = 1; // turn on the I2C1 module
}

void i2c_master_start(void) {
    I2C1CONbits.SEN = 1; // send the start bit
    while (I2C1CONbits.SEN) {
        ;
    } // wait for the start bit to be sent
}

void i2c_master_restart(void) {
    I2C1CONbits.RSEN = 1; // send a restart 
    while (I2C1CONbits.RSEN) {
        ;
    } // wait for the restart to clear
}

void i2c_master_send(unsigned char byte) { // send a byte to slave
    I2C1TRN = byte; // if an address, bit 0 = 0 for write, 1 for read
    while (I2C1STATbits.TRSTAT) {
        ;
    } // wait for the transmission to finish
    if (I2C1STATbits.ACKSTAT) { // if this is high, slave has not acknowledged
        // ("I2C1 Master: failed to receive ACK\r\n");
        while(1){} // get stuck here if the chip does not ACK back
    }
}

unsigned char i2c_master_recv(void) { // receive a byte from the slave
    I2C1CONbits.RCEN = 1; // start receiving data
    while (!I2C1STATbits.RBF) {
        ;
    } // wait to receive the data
    return I2C1RCV; // read and return the data
}

void i2c_master_ack(int val) { // sends ACK = 0 (slave should send another byte)
    // or NACK = 1 (no more bytes requested from slave)
    I2C1CONbits.ACKDT = val; // store ACK/NACK in ACKDT
    I2C1CONbits.ACKEN = 1; // send ACKDT
    while (I2C1CONbits.ACKEN) {
        ;
    } // wait for ACK/NACK to be sent
}

void i2c_master_stop(void) { // send a STOP:
    I2C1CONbits.PEN = 1; // comm is complete and master relinquishes bus
    while (I2C1CONbits.PEN) {
        ;
    } // wait for STOP to complete
}

void setMCP23008pin(unsigned char byte, unsigned char val) {
    i2c_master_start();
    i2c_master_send(0b01000000);
    i2c_master_send(byte);
    i2c_master_send(val);
    i2c_master_stop();
}

unsigned char getMCP23008pin(unsigned char byte) {
    unsigned char input;
    i2c_master_start();
    i2c_master_send(0b01000000);
    i2c_master_send(byte);
    i2c_master_restart();
    i2c_master_send(0b01000001);
    input = i2c_master_recv();
    i2c_master_ack(1);
    i2c_master_stop();
    return input;
}