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
#include <htc.h>
#include "I2C_slave.h"

unsigned int UMS_info ;

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
#define TIME_OVER  50000      // 超音波センサーから無返答時のタイムアウト時間(50ms)
#define T1COUT     15536      // タイマー１用カウントの初期値(65536-50000:50msカウント)

void init();

int cm = 0;
int mm = 0;
int i = 0;
int i2c_ans = 0;
int pin;

int main(void) {
    init();
    I2C_init();
    OPTION_REG = 0b00000000 ; // デジタルI/Oに内部プルアップ抵抗を使用する
    
    // 反射波時刻カウント用タイマー１の設定(1カウントは1us)
     T1CON   = 0b01110000 ;   // クロックソースはFosc、プリスケーラ1/8、
     TMR1H   = (T1COUT >> 8) ;// 30000(30ms)までカウントさせる
     TMR1L   = (T1COUT & 0x00ff) ;
     TMR1IF  = 0 ;            // TIMER1のフラグを0にする
     TMR1IE  = 1 ;            // TIMER1の割込みを許可する
     // ＤＡＣの設定(約2.0Vをコンパレータの閾値とする)
     DACCON0 = 0b11000000 ;   // VDD/VSSを使用、DACOUTピン(RA2)使わない
     DACCON1 = 13 ;           // 約2.0Vを出力( 5V*(13/2^5)=2.03125 )
     // コンパレータ２の設定(割込みで利用)
//     CM2CON0 = 0b10010110 ;   // －＞＋でON、高速モード、出力は反転、ヒステリシス有効
//     CM2CON1 = 0b10010001 ;   // 立上りで割込み利用、＋はDAC入力、－はRA1から入力
//     C2IF    = 0 ;            // コンパレータ２割込フラグを0にする
     ADIF    = 0 ;
//     C2IE    = 1 ;            // コンパレータ２割込みを許可する
     ADIE    = 1 ;
     // 周辺装置全体の割り込みを許可
     PEIE    = 1 ;            // 周辺装置割り込み有効
     GIE     = 1 ;            // 全割込み処理を許可する

    while (1) {
        cm = PalseSonarRead_cm();
        cm = cm * 10;
        if((cm != 0) && (cm <= 300)){
            i2c_ans = cm;
        }else{
            mm = PalseSonarRead_mm();
            i2c_ans = mm;
        }
        send_data[0] = i2c_ans % 0x100;     //dat1 = (char)data;
        send_data[1] = i2c_ans / 0x100;     //dat2 = (char)data >> 8;
        //data = dat2 * 0x100 + dat1; 読み出しの際
        __delay_ms(30);
    }

    return (0);
}

void init() {
    OSCCONbits.IRCF = 0b1111; //Set oscillator 16MHz
    ANSELA = 0x00; //Set RA pins digital
    TRISA4 = 0;
    TRISA5 = 1;

    PORTA = 0x00; //Set PORTA Low
    return;
}

static void interrupt forinterrupt(){
    #include "I2C_slave_int.h"
    // コンパレータ関連の割込み処理
    // センサーから返答があった場合の処理(物体からの反射有り)
    if (ADIF == 1) {
        TMR1ON = 0 ;                       // TMR1カウント停止
        UMS_info = TMR1L ;                 // カウント値を記録する
        UMS_info = UMS_info | (TMR1H << 8) ;
        UMS_info = UMS_info - T1COUT ;
        TMR1H = (T1COUT >> 8) ;            // カウント値の再設定
        TMR1L = (T1COUT & 0x00ff) ;
        ADIF = 0 ;                         // コンパレータ割込フラグをリセット
    }
    // タイマー１の割込み処理
    // センサーから返答がない場合の処理(物体からの反射がない、近くに物体が無い)
    if (TMR1IF == 1) {
        TMR1ON = 0 ;                       // TMR1カウント停止
        UMS_info = TIME_OVER ;             // カウント値は時間切れ
        TMR1H = (T1COUT >> 8) ;            // カウント値の再設定
        TMR1L = (T1COUT & 0x00ff) ;
        TMR1IF = 0 ;                       // タイマー1割込フラグをリセット
    }
}

int PalseSonarRead_cm(){
    int leng;
    int ans_cm;
    
    TRISA4 = 0; //出力
    RA4 = 0;    //LOW
    __delay_ms(2);
    RA4 = 1;
    __delay_ms(5);
    RA4 = 0;
    
    TRISA4 = 1;
    leng = pulseIn(4);      //high状態の長さ(マイクロ秒)
    if(leng < 18000){
        ans_cm = (leng / 29) / 2;
    }else ans_cm = 0;
    return ans_cm;
}

int PalseSonarRead_mm(){
    unsigned long ans_mm = pulseIn(5);
	return ans_mm;
}

int pulseIn(pin){
    long t ;
    int ans ;
    
    ans = 0;
    if(pin == 4){
        while(RA4 == 1);
        while(RA4 == 0);
        TMR1ON = 1 ;            // TMR1カウント開始
        UMS_info = 0 ;
        while(UMS_info == 0);
        if (UMS_info < TIME_OVER){
            ans = UMS_info;
        }
        return ans;
    }
    
    if (pin == 5){
        while(RA5 == 1);
        while(RA5 == 0);
        TMR1ON = 1 ;            // TMR1カウント開始
        UMS_info = 0 ;
        while(UMS_info == 0);
        if (UMS_info < TIME_OVER){
            ans = UMS_info;
        }
        return ans;
    }
}
