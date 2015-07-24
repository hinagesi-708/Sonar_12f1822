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


int cm = 0;                //
int mm = 0;                //
int i = 0;                          //
int i2c_ans = 0;           //

void init();

int main(void) {
    
    // 反射波時刻カウント用タイマー１の設定(1カウントは1us)
     T1CON   = 0b01110000 ;   // クロックソースはFosc、プリスケーラ1/8、
     TMR1H   = (T1COUT >> 8) ;// 30000(30ms)までカウントさせる
     TMR1L   = (T1COUT & 0x00ff) ;
     TMR1IF  = 0 ;            // TIMER1のフラグを0にする
     TMR1IE  = 1 ;            // TIMER1の割込みを許可する
    
    init();
    I2C_init();

    while (1) {
        cm = PalseSonarRead_cm();
        cm = cm * 10;
//        if(cm >= 230){
//            cm = cm + 10;
//        }
        if((cm != 0) && (cm <= 300)){
            i2c_ans = cm;
        }else{
            mm = PalseSonarRead_mm();
//            mm = mm - (mm * (-0.01749) + 28.78306);
            i2c_ans = mm;
        }
        send_data[0] = i2c_ans % 0x100;     //dat1 = (char)data;
        send_data[1] = i2c_ans / 0x100;     //dat2 = (char)data >> 8;
        //data = dat2 * 0x100 + dat1; 読み出しの際
        
        // 20℃と言う事でセンサーから距離を読込む、20mm程誤差が有るので足して置く
//          val = UsonicMeasurRead(20,20) ;
        
    __delay_ms(30);
    }

    return (0);
}

void init() {
    OSCCONbits.IRCF = 0b1111;       //内部クロック16MHzで駆動
    ANSELA  = 0x00;                 //全て0:デジタルI/Oとする
//    TRISA4 = 0;                     //cm OUT
//    TRISA5 = 1;                     //mm IN
    TRISA = 0b00100110;

    PORTA = 0x00;                   //PORTAの中身をきれいにする
    return;
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
    leng = pulseIn(4);      //high状態の長さ(マイクロ秒)
    if(leng < 18000){
        ans_cm = (leng / 29) / 2;
    }else ans_cm = 0;
    return ans_cm;
}

int PalseSonarRead_mm(){
    unsigned long ans_pw = pulseIn(5);
	return ans_pw;
}
int pin;
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

static void interrupt forinterrupt(){
    #include "I2C_slave_int.h"
}



void interrupt InterFunction( void )
{
     // コンパレータ関連の割込み処理
     // センサーから返答があった場合の処理(物体からの反射有り)
     if (C1IE == 1) {
          TMR1ON = 0 ;                       // TMR1カウント停止
          UMS_info = TMR1L ;                 // カウント値を記録する
          UMS_info = UMS_info | (TMR1H << 8) ;
          UMS_info = UMS_info - T1COUT ;
          TMR1H = (T1COUT >> 8) ;            // カウント値の再設定
          TMR1L = (T1COUT & 0x00ff) ;
          C1IE = 0 ;                         // コンパレータ割込フラグをリセット
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





/*// 返答時間から距離を求める
     if (UMS_info < TIME_OVER) {
          t = 331500 + (600 * temp) ;   // 音波の伝搬する速度を求める
          t = (t * UMS_info) / 1000000 ;// 距離の計算
          ans = t / 2 ;                 // 往復なので÷2
          ans = ans + correction ;      // 距離の補正値を加える
     }
     return ans ;                       // mmの距離を返す
*/
