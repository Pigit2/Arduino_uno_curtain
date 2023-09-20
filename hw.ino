#include "SoftwareSerial.h"
#include "U8glib.h"
#include "DHT.h"
#include "newfont.h"
#include "Servo.h"

#define DHTPIN 2
#define DHTTYPE DHT11

Servo myservo;
SoftwareSerial mySerial(5, 11);
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0);
DHT dht(DHTPIN, DHTTYPE);

unsigned int mode=0,mp=1,ap=1,run=1;//模式标志，手动页标志，自动标志，开关状态标志：窗帘默认打开


void setup() {

  Serial.begin(115200);
  dht.begin();
  myservo.attach(9);
  pinMode(3, INPUT);
  
  u8g.firstPage();
  do {
    StartPage();
  } while( u8g.nextPage() );//初始化页

  esp8266();
  
  u8g.firstPage();
  do {
    FinishPage();
  } while( u8g.nextPage() );//完成提示页
  delay(1500);

  u8g.firstPage();
  do {
    HomePage();
  } while( u8g.nextPage() );//信息页
  
}

void loop() {
 
  ReceiveData();
  
}





void StartPage(){
  u8g.drawBitmapP(16,0,2,16,font_1);
  u8g.drawBitmapP(32,0,2,16,font_2);
  u8g.drawBitmapP(48,0,2,16,font_3);
  u8g.drawBitmapP(64,0,2,16,font_4);
  u8g.drawBitmapP(80,0,2,16,font_5);//正在初始化
  u8g.setFont(u8g_font_gdb11);
  u8g.drawStr(0, 15, "...");
  u8g.drawStr(96, 15, "...");
}

void FinishPage(){
  u8g.setFont(u8g_font_gdb11);
  u8g.drawStr(0, 16, "........................");
  u8g.drawStr(32, 48, "finish!");
}

void HomePage(){
  u8g.drawXBMP(0,0,128,64,bmp_1);//主页
}

void SensorPage(){
  int h = dht.readHumidity()-3;
  int t = dht.readTemperature();
  //float f = dht.readTemperature(true);

  // if (isnan(h) || isnan(t)) {
  //   Serial.println(F("Failed to read from DHT sensor!"));
  //   return;
  // }

  // Compute heat index in Fahrenheit (the default)
  //float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  //float hic = dht.computeHeatIndex(t, h, false);
  
  Serial.print(F("湿度："));
  Serial.print(h);
  Serial.print(F("%  温度："));
  Serial.print(t);
  Serial.println(F("°C"));
  // Serial.print(f);
  // Serial.print(F("°F \n"));

  u8g.setFont(u8g_font_gdb11);
  u8g.drawBitmapP(28,0,2,16,font_6);
  u8g.drawBitmapP(44,0,2,16,font_7);
  u8g.drawBitmapP(60,0,2,16,font_8);
  u8g.drawBitmapP(76,0,2,16,font_9);//当前环境
  u8g.drawBitmapP(16,22,2,16,font_10);
  u8g.drawBitmapP(32,22,2,16,font_11);
  u8g.drawBitmapP(48,22,2,16,font_13);//温度
  u8g.setPrintPos(64,36);
  u8g.print(t);
  u8g.drawBitmapP(87,23,2,16,font_18);//度
  u8g.drawBitmapP(16,44,2,16,font_12);
  u8g.drawBitmapP(32,44,2,16,font_11);
  u8g.drawBitmapP(48,44,2,16,font_13);//湿度
  u8g.setPrintPos(64,58);
  u8g.print(h);
  u8g.setPrintPos(88,58);
  u8g.print("%");
  delay(200);
}

void ManualPage(){
  u8g.drawBitmapP(28,0,2,16,font_26);
  u8g.drawBitmapP(44,0,2,16,font_15);
  u8g.drawBitmapP(60,0,2,16,font_16);
  u8g.drawBitmapP(76,0,2,16,font_17);//手动模式
  if(run==1){
    u8g.drawBitmapP(40,32,2,16,font_27);
    u8g.drawBitmapP(56,32,2,16,font_28);
    u8g.drawBitmapP(72,32,2,16,font_21);//已打开
  }
  if(run==0){
    u8g.drawBitmapP(40,32,2,16,font_27);
    u8g.drawBitmapP(56,32,2,16,font_24);
    u8g.drawBitmapP(72,32,2,16,font_25);//已关闭
  }
}

void AutoPage(){
  u8g.drawBitmapP(28,0,2,16,font_14);
  u8g.drawBitmapP(44,0,2,16,font_15);
  u8g.drawBitmapP(60,0,2,16,font_16);
  u8g.drawBitmapP(76,0,2,16,font_17);//自动模式
  u8g.drawBitmapP(68,22,2,16,font_28);
  u8g.drawBitmapP(84,22,2,16,font_21);//开启
  u8g.drawBitmapP(68,44,2,16,font_24);
  u8g.drawBitmapP(84,44,2,16,font_25);//关闭
  if(ap==1){
    u8g.drawBitmapP(24,22,2,16,font_19);
    u8g.drawBitmapP(40,22,2,16,font_20);//夜间
    u8g.drawBitmapP(24,44,2,16,font_23);
    u8g.drawBitmapP(40,44,2,16,font_20);//日间
  }
  if(ap==2){
    u8g.drawBitmapP(24,22,2,16,font_23);
    u8g.drawBitmapP(40,22,2,16,font_20);//日间
    u8g.drawBitmapP(24,44,2,16,font_19);
    u8g.drawBitmapP(40,44,2,16,font_20);//夜间
  }
}




boolean SendCommand(String cmd, String ack){
  mySerial.println(cmd); // Send "AT+" command to module
  if (!echoFind(ack)) // timed out waiting for ack string
    return true; // ack blank or ack found
}

boolean echoFind(String keyword){
  byte current_char = 0;
  byte keyword_length = keyword.length();
  long deadline = millis() + 5000;
  while(millis() < deadline){
    if (mySerial.available()){
      char ch = mySerial.read();
      Serial.write(ch);
      if (ch == keyword[current_char])
        if (++current_char == keyword_length){
        Serial.println();
        return true;
      }
    }
  }
  return false; // Timed out
}


void esp8266(){
  Serial.println("start!");
  mySerial.begin(115200);
  // mySerial.println("AT\r\n");
  // delay(500);
  // mySerial.println("AT+RST\r\n");
  // delay(3000);
  // mySerial.println("AT+CWMODE=3\r\n");
  // delay(500);
  // mySerial.println("AT+CWSAP=\"ESP8266\",\"12345678\",1,3\r\n");
  // delay(6000);
  // mySerial.println("AT+CWLIF\r\n");
  // delay(500);
  // mySerial.println("AT+CIPMUX=1\r\n");
  // delay(500);
  // mySerial.println("AT+CIPSERVER=1,8080\r\n");
  // delay(1000);
  SendCommand("AT+RST", "Ready");
  delay(3000);
  SendCommand("AT+CWMODE=3","OK");
  delay(500);
  SendCommand("AT+CWSAP=\"ESP8266\",\"12345678\",1,3","OK");
  delay(1500);
  SendCommand("AT+CWLIF", "OK");
  delay(500);
  SendCommand("AT+CIPMUX=1","OK");
  delay(500);
  SendCommand("AT+CIPSERVER=1,8080","OK");
  delay(500);
  Serial.println("\nfinish!");
}



void ReceiveData(){
  String ReceiveString="";
  boolean Ready = false;
  //int Light = digitalRead(3);

  while (mySerial.available()){
    ReceiveString=mySerial.readString();
    Ready= true;
  }

  while (Serial.available()){
    ReceiveString=Serial.readString();
    Ready= true;
  }

  if (Ready) {
    Serial.println("Received String: " + ReceiveString);
  }

  if (ReceiveString.indexOf("aaa") != -1) {//传感器页
    mode=1;
  }


  if (ReceiveString.indexOf("bbb") != -1) {//手动模式
    mode=2;
    u8g.firstPage();
    do {
      ManualPage();
    } while( u8g.nextPage() );//
  }


  if (ReceiveString.indexOf("ccc") != -1) {//自动模式
    mode=3;
  }

//-----------------------------------------------------------------------------------语音模式

  // if (ReceiveString.indexOf("ggg") != -1 && run==0) {//语音开
  //   Serial.println("Reon");
  //   myservo.writeMicroseconds(700); //顺时针旋转,500~1500顺时针旋转，值越小，旋转速度越大
  //   delay(2000);
  //   myservo.writeMicroseconds(1500);//1500控制舵机停止
  //   run=1;
  // }
  // if (ReceiveString.indexOf("hhh") != -1 && run==1) {//语音关
  //   Serial.println("Reof");
  //   myservo.writeMicroseconds(2100);//逆时针旋转,1500~2500逆时针旋转，值越大，旋转速度越大
  //   delay(2000);
  //   myservo.writeMicroseconds(1500);//1500控制舵机停止
  //   run=0;
  // }


  //-----------------------------------------------------------------------------------温湿度
  if (mode==1) {
    u8g.firstPage();
    do {
      SensorPage();
    } while( u8g.nextPage() );//
  }

  //-----------------------------------------------------------------------------------手动模式2
  
  if (mode==2) {
    if (ReceiveString.indexOf("ddd") != -1 && run==0) {//手动开
      //digitalWrite(LED,HIGH);
      Serial.println("Reon");
      run=1;
      u8g.firstPage();
      do {
        ManualPage();
      } while( u8g.nextPage() );//已打开页
      myservo.writeMicroseconds(700); //顺时针旋转,500~1500顺时针旋转，值越小，旋转速度越大
      delay(2000);
      myservo.writeMicroseconds(1500);//1500控制舵机停止
    }
    if (ReceiveString.indexOf("eee") != -1 && run==1) {//手动关
      //digitalWrite(LED,LOW);
      Serial.println("Reof");
      run=0;
      u8g.firstPage();
      do {
        ManualPage();
      } while( u8g.nextPage() );//已关闭页
      myservo.writeMicroseconds(2100);//逆时针旋转,1500~2500逆时针旋转，值越大，旋转速度越大
      delay(2000);
      myservo.writeMicroseconds(1500);//1500控制舵机停止
    }
    if (ReceiveString.indexOf("ggg") != -1 && run==0) {//手动开
      //digitalWrite(LED,HIGH);
      Serial.println("Reon");
      run=1;
      u8g.firstPage();
      do {
        ManualPage();
      } while( u8g.nextPage() );//已打开页
      myservo.writeMicroseconds(700); //顺时针旋转,500~1500顺时针旋转，值越小，旋转速度越大
      delay(2000);
      myservo.writeMicroseconds(1500);//1500控制舵机停止
    }
    if (ReceiveString.indexOf("hhh") != -1 && run==1) {//手动关
      //digitalWrite(LED,LOW);
      Serial.println("Reof");
      run=0;
      u8g.firstPage();
      do {
        ManualPage();
      } while( u8g.nextPage() );//已关闭页
      myservo.writeMicroseconds(2100);//逆时针旋转,1500~2500逆时针旋转，值越大，旋转速度越大
      delay(2000);
      myservo.writeMicroseconds(1500);//1500控制舵机停止
    }
  }

  //-----------------------------------------------------------------------------------自动模式

  if(mode==3){
    if (ReceiveString.indexOf("fff") != -1){//自动模式切换
      if(ap==1)
      {
        ap=2;
        Serial.println("ap=2");
      }
      else{
        ap=1;
        Serial.println("ap=1");
      }
    }
      
    u8g.firstPage();
    do {
      AutoPage();
    } while( u8g.nextPage() );//

    if(ap==1){//夜间打开，日间关闭
      if(digitalRead(3)==1 && run==0){
        Serial.println("Reoff");
        myservo.writeMicroseconds(700);
        delay(2000);
        myservo.writeMicroseconds(1500);
        run=1;
      }
      if(digitalRead(3)==0 && run==1){
        Serial.println("Reon");
        myservo.writeMicroseconds(2100);
        delay(2000);
        myservo.writeMicroseconds(1500);
        run=0;
      }
    }
  
  
    if(ap==2){//夜间关闭，日间打开
      if(digitalRead(3)==1 && run==1){
        Serial.println("Reon");
        myservo.writeMicroseconds(2100);
        delay(2000);
        myservo.writeMicroseconds(1500);
        run=0;
      }
      if(digitalRead(3)==0 && run==0){
        Serial.println("Reof");
        myservo.writeMicroseconds(700);
        delay(2000);
        myservo.writeMicroseconds(1500);
        run=1;
      }
    }
  }

}
