
    static int charadd = 0;
    char dummy;
    if(SSP1IF){
        if(SSP1STATbits.R_nW){
            if(SSP1STATbits.BF){
                charadd = 0;
                dummy = SSP1BUF;
                while(SSP1STATbits.BF);
                SSP1BUF = send_data[0];
                charadd++;
            }else{
                if(SSP1CON2bits.ACKSTAT == 0){
                    SSP1BUF = send_data[charadd];
                    charadd++;
                }else{
                    charadd = 0;
                }
            }
         }else{
            if(SSP1STATbits.D_nA){
                reseaved_data[charadd] = SSPBUF;
                charadd++;
                reseaved_data[charadd] = '\0';
            }else{
                dummy = SSP1BUF;
                charadd = 0;
            }
         }
        SSP1IF = 0;
        SSP1CON1bits.CKP = 1;
    }
