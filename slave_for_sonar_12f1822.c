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
 * (RA5)mmSonar---|2       7|---(RA0)       *
 * (RA4)cmSonar---|3       6|---SCL(RA1)    *
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
int cm = 0;
int mm = 0;
int i2c_ans = 0;
int pin;

void init();

int main(void) {
    init();
    I2C_init();

    while (1) {
        cm = PalseSonarRead_cm();
        cm = cm * 10;
        if ((cm != 0) && (cm <= 300)){
            i2c_ans = cm;
        }else{
            mm = PalseSonarRead_mm();
//            mm = 800;
            i2c_ans = mm;
        }
        __delay_ms(60);
        send_data[0] = i2c_ans % 0x100;     //dat1 = (char)data;
        send_data[1] = i2c_ans / 0x100;     //dat2 = (char)data >> 8;
        //data = dat2 * 0x100 + dat1; 読み出しの際
//        send_data[0] = 7;
//        send_data[1] = 9;
    }

    return (0);
}

void init() {
    OSCCONbits.IRCF = 0b1111;       //Set oscillator 16MHz
    ANSELA  = 0x00;                 //Set RA pins digital
    
    TRISA4 = 1;
    TRISA5 = 1;
    IOCAP4 = 1;
    IOCAP5 = 1;
    INTCONbits.IOCIE = 1;
    IOCAF4 = 0;
    IOCAF5 = 0;
    INTCONbits.GIE = 1;

    PORTA = 0x00;                   //Set PORTA Low
    return;
}

static void interrupt forinterrupt(){
    #include "I2C_slave_int.h"
}

int PalseSonarRead_cm(){
    	long leng;
	int ans_cm;

	TRISA4 = 0;
    	RA4 = 0;
    	__delay_ms(2);
    	RA4 = 1;
    	__delay_ms(5);
    	RA4 = 0;

	TRISA4 = 1;
	leng = pulseIn(4);
	if(leng < 18000){
		ans_cm = (leng / 29) / 2;
	}else ans_cm = 0;

	return ans_cm;
}
int PalseSonarRead_mm(){
    int ans_mm = pulseIn(5);
	return ans_mm;
}

int pulseIn(pin){
    long timer = 0;
    if(pin == 4){
        while(RA4 == 1);
        while(RA4 == 0);
        while(RA4 == 1){
            __delay_us(1);
            timer++;
            if(timer > 20000) break;
        }
    }else if(pin == 5){
        while(RA5 == 1);
        while(RA5 == 0);
        while(RA5 == 1){
            __delay_us(1);
            timer++;
            if(timer > 5000) break;
        }
    }return timer;
}
