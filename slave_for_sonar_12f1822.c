
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
    
    int cm;
    int mm;
    int i2c_ans;

    while (1) {
        cm = 0;
        mm = 0;
        i2c_ans = 0;
        cm =  Pls_cm();
        cm = cm * 10;
        if ((cm != 0) && (cm <= 300)){
            i2c_ans = cm;
        }else{;
            mm = Pls_mm();
            i2c_ans = mm;
        }
        
    }
    send_data[0] = i2c_ans % 0x100;     //dat1 = (char)data;
    send_data[1] = i2c_ans / 0x100;     //dat2 = (char)data >> 8;
    //data = dat2 * 0x100 + dat1; 読み出しの際
    return (0);
    __delay_ms(100);
}

void init() {
    OSCCONbits.IRCF = 0b1111;       //Set oscillator 16MHz
    ANSELA  = 0x00;                 //Set RA pins digital
    TRISA4 = 0;
    TRISA5 = 0;

    PORTA = 0x00;                   //Set PORTA Low
    return;
}
int Pls_cm() {
    int leng_cm;
    
    TRISA0 = 0;
    RA0 = 0;
    __delay_us(2);
    RA0 = 1;
    __delay_us(5);
    RA0 = 0;
    
    TRISA0 = 1;
    leng_cm = PulseIn_cm();
    
    return leng_cm;
}
int PulseIn_cm(){
    long time_cm = 0;
    while(RA0 == 1);
    while(RA0 == 0);
    while(RA0 == 1){
        __delay_us(1);
        time_cm ++;
        if(time_cm > 20000) break;
    }
    time_cm = time_cm / 58;
    return time_cm;
}

int Pls_mm(){
    long time_mm = 0;
    while(RA5 == 1);
    while(RA5 == 0);
    while(RA5 == 1){
        __delay_us(1);
        time_mm ++;
        if(time_mm > 5000) break;
    }
    return time_mm;
}

static void interrupt forinterrupt(){
    #include "I2C_slave_int.h"
}
