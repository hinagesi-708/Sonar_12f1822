#include <Wire.h>
/*
SCL:A5
SDA:A4
*/
#define ADDRESS 13  //通信するスレーブのアドレス
                    //複数スレーブと通信する際は最初のスレーブのアドレス
                    //※実験の都合上複数アドレスと通信するときは連番にしてあります
                    //  実際は連番である必要はないです

#define SLAVE   1   //通信するスレーブ数

int8_t send_data[8] = {0};
//int8_t reseaved_data[8] = {0};
unsigned int reseaved_data[8] = {0};

int ans = 0;

void setup() {
	Wire.begin();//I2Cラインにマスターとして参加
	Serial.begin(9600);//シリアル通信を開始
        Serial.println("start");//start送信
        reseaved_data[0] = 100;
        reseaved_data[1] = 100;
}

void loop() {
    while(1){
/*
    //送信部分
    //#####################################################################################
      Wire.beginTransmission(ADDRESS); //アドレスADDRESSにデータ送信を開始する
        for(int i = 0;send_data[i] != 0 && i < 8;i++){  
          Wire.write(send_data[i]); //送信スタックにデータを並べる   
          
          Serial.print("send_data[");
          Serial.print(i+0);
          Serial.print("]:");
          Serial.println(send_data[i]+0);
          //send_data[i]:結果
          //という形式で送ったデータを表示
          //+0しているのは数値として表示するため
        }
      Wire.endTransmission(true); //スタック内のデータを送信
      Serial.println();
        delay(100); //PICがデータを用意する時間待機（100msは正直長い）
    //#####################################################################################
*/
    //受信部分
    //#####################################################################################
      for(int addr = ADDRESS;addr < ADDRESS+SLAVE;addr++){
        //最初のアドレスからスレーブ数分だけ受信を繰り返す

        ans = Wire.requestFrom(addr,2); //アドレスaddrに3byteを要求
         //Wire.requestFromの返り値は要求したデータの個数になります
        //なので、requestFromの返り値をansに入れることで
        //直後のfor文で何回Wire.readをするかを判定します
        
        for(int count = 0;count < ans;count++){
          reseaved_data[count] = Wire.read();//受信した文字列を格納
          
          Serial.print("reseaved_data(");
          Serial.print(addr+0);
          Serial.print(")[");
          Serial.print(count+0);
          Serial.print("]:");
          Serial.println(reseaved_data[count]+0);
          //reseaved_data(アドレス)[i]:結果
          //という形式で受信したデータを表示
          //+0しているのは数値として表示するため
        }Serial.println();
        
//        data = dat2 * 0x100 + dat1; 読み出しの際
        int hogehoge;
        hogehoge = reseaved_data[1]*0x100+reseaved_data[0];
        Serial.print(hogehoge);
        
      }Serial.println();
    //#####################################################################################

	delay(1000);

    }
}
