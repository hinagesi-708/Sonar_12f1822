#include <xc.h>
#include "I2C_slave.h"

void I2C_init(){
    SDA(TRIS) = 1;
    SCL(TRIS) = 1;

    PIE1bits.SSP1IE = 1;
    PIE2bits.BCL1IE = 1;
    INTCONbits.PEIE = 1;
    INTCONbits.GIE  = 1;

    PIR1bits.CCP1IF = 0;
    PIR2bits.BCL1IF = 0;

    SSPSTATbits.SMP = 1;

    SSP1ADD = I2C_ADDR_DEV1 << 1;
    SSP1MSK = 0b11111110;

    SSPCON2bits.GCEN = 0;
    SSPCON2bits.SEN = 1;
    SSPCON1bits.CKP = 1;

    SSPCON1bits.SSPM = 0b0110;

    SSPCON1bits.SSPEN = 1;
}

void set_send_string(signed char Str[8]){
    for(int i = 0;i < 8;i++){
        send_data[i] = Str[i];
    }return;
}
