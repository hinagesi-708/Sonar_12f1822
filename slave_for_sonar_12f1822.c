
/* 
 * File:   slave_for_sonar_12f1822.c
 * Author: kayoko
 *
 * Created on 2015/07/23, 14:14
 */
//  http://www.geocities.jp/zattouka/GarageHouse/micon/MPLAB/16F1827/USS/USS.htm

/********************************************
 *  compiler    MPLAB XC8(v1.34)            *
 *  PIC         PIC12F1822                  *
 *  clock        16MHz(INTIO)               *
 *                                          *
 *  use_port                                *
 *                __________                *
 *          Vdd---|1  ●   8|---Vss         *
 * (RA5)mmSonar---|2       7|---cmSonar(RA0)*
 *        (RA4)---|3       6|---SCL(RA1)    *
 *      (RA3)×---|4       5|---SDA(RA2)    *
 *                ==========                *
 ********************************************/

#include <xc.h>
#include "I2C_slave.h"

#pragma config CLKOUTEN = OFF
#pragma config WDTE     = OFF
#pragma config PWRTE    = ON
#pragma config CP       = OFF
#pragma config BOREN    = ON
#pragma config FCMEN    = OFF
#pragma config MCLRE    = ON
#pragma config CPD      = OFF
#pragma config IESO     = OFF
#pragma config FOSC     = INTOSC

#pragma config LVP      = ON

#define _XTAL_FREQ 16000000

void init();

int main(void) {
    init();
    I2C_init();
    
//    int cm = 0;
    int mm = 0;
    int i2c_ans = 0;

    while (1) {
//        i2c_ans = 800;      //test用コード
        i2c_ans = Pls_mm();
        send_data[0] = i2c_ans % 0x100;     //dat1 = (char)data;
        send_data[1] = i2c_ans / 0x100;     //dat2 = (char)data >> 8;

    }

    return (0);
}

void init() {
    OSCCONbits.IRCF = 0b1111;       //Set oscillator 16MHz
    ANSELA  = 0x00;                 //Set RA pins digital
    TRISA4 = 0;
    TRISA5 = 0;

    PORTA = 0x00;                   //Set PORTA Low
    return;
}

int Pls_mm(){
    int i;
    while(RA5 == 1);
    while(RA5 == 0);
    for(i = 0; RA5 == 1; i++);
    return i;
}

static void interrupt forinterrupt(){
    #include "I2C_slave_int.h"
}
