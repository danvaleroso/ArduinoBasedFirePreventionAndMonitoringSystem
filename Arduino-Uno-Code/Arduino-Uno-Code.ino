#include <dht.h> //temp sensor library
#include <LiquidCrystal_I2C.h> //LCD library
#include <Wire.h> //I2C library

#define mq2Sensor A0
#define DHT11_PIN 12
#define alarm 8
#define button 9     
#define ledalarm 10

dht DHT; 
LiquidCrystal_I2C lcd(0x3F, 16 , 2);

char phone_no[]="09951745916";//phone no. of the owner 
String gsmData;
String getNum;
int mq2Value;
int tempValue;
int ledState = LOW; 
// use for millis();
unsigned long long int sensorReading;
unsigned long long int sendInterval;
unsigned long long int previousMillis;

//function for sending SMS
void sendSMS(String number,String message){
    number = "AT+CMGS=\""+number+"\"\r\n";
    Serial.print(number);
    gsmWaitResponse();
    Serial.print(message);
    Serial.write((char)26);
    gsmWaitResponse(); 
}
//function for gsm response
void gsmWaitResponse(){
  unsigned long long int prevTime = millis();
  while(!Serial.available()||((millis()-prevTime)<=2000)){}
  gsmData = Serial.readString();  
}

void sendback(String message){
  
    gsmData = Serial.readString();
    if(gsmData.indexOf("+CMT:")>-1){
      int gsmIndex = gsmData.indexOf("+CMT:");
      int gsmLength = gsmData.length();
      // get the number of sender 
      // send UPDATE to the sender
      getNum = gsmData.substring(gsmIndex+7,gsmIndex+20);
      sendSMS(getNum,message);
}
  }
void setup() {
  
//lcd setup
lcd.init();
lcd.backlight(); 

lcd.print("Initializing.");
//temperature setup
int chk = DHT.read11(DHT11_PIN);
tempValue = DHT.temperature;
pinMode(button, INPUT);
pinMode(alarm, OUTPUT);
pinMode(ledalarm, OUTPUT);
//GSM module setup
Serial.begin(9600);
delay(300);
Serial.write("ATE0\r\n");// echo off
gsmWaitResponse();
lcd.print(".");
Serial.write("AT+CMGF=1\r\n");// set to SMS text mode
gsmWaitResponse();
lcd.print(".");
Serial.write("AT+CMGDA=\"DEL ALL\"\r\n");// delete sim messages
gsmWaitResponse();
lcd.print(".");
sendSMS(phone_no,"System Ready!\nMQ-2 Sensor: "+String(analogRead(A0))+"\nTemperature: "+String(tempValue)+" degree Celcius");
lcd.clear();
lcd.print("System Ready!");
digitalWrite(alarm,HIGH);
digitalWrite(ledalarm,HIGH);
delay(200);
digitalWrite(alarm,LOW);
digitalWrite(ledalarm,LOW);
delay(200);
digitalWrite(alarm,HIGH);
digitalWrite(ledalarm,HIGH);
delay(200);
digitalWrite(alarm,LOW);
digitalWrite(ledalarm,LOW);
delay(200);
}

void loop() {
  
mq2Value = analogRead(A0);
//printing temp and smoke per .1 sec without delay
if((millis()-sensorReading)>=100){
    int chk = DHT.read11(DHT11_PIN);
    tempValue = DHT.temperature;
    lcd.setCursor(0,0);
    lcd.print("Temp: ");
    lcd.print(tempValue);
    lcd.print(" Celcius ");
    lcd.setCursor(0,1);
    lcd.print("MQ 2: ");
    lcd.print(analogRead(A0));
    lcd.print("      ");
    sensorReading = millis();  
  }
//alarm will activate if...
if(mq2Value>=800||DHT.temperature>=70){
    
    if((millis()-sendInterval)>=5000){
      
      sendSMS(phone_no,"FIRE/SMOKE: DETECTED!!\nMQ-2 Sensor: "+String(analogRead(A0))+"\nTemperature: "+String(tempValue)+" degree Celcius"); 
      sendInterval = millis();
      lcd.clear();
      lcd.println("Fire/smoke:     ");
      lcd.setCursor(0,1);
      lcd.println("DETECTED        ");
      
    }
    //alarm will not stop unless the button is pressed or SMS received
    int val = 0;
    while (val<1){
      digitalWrite(alarm, HIGH);
      
      //blinking LED without Delay  
      unsigned long currentMillis = millis();
    
      if (currentMillis - previousMillis >= 100) {
        previousMillis = currentMillis;
        if (ledState == LOW) {
        ledState = HIGH;
         } else {
         ledState = LOW;
        }
       }
       digitalWrite(ledalarm, ledState);
  
      //alarm will stop if the button is HIGH/pressed
      if (digitalRead(button)==HIGH){
        val++;
        digitalWrite(alarm,LOW);
        digitalWrite(ledalarm,LOW);
      }
      //alarm will stop if there's SMS received
      if(Serial.available()){
        digitalWrite(alarm,LOW);
        digitalWrite(ledalarm,LOW);
        lcd.clear();
        lcd.print("ALARM TERMINATED");
        val++;
        gsmData = Serial.readString();
        if(gsmData.indexOf("+CMT:")>-1){
          int gsmIndex = gsmData.indexOf("+CMT:");
          int gsmLength = gsmData.length();
          // get the number of sender 
          // send notif to the Sender
          getNum = gsmData.substring(gsmIndex+7,gsmIndex+20);
          sendSMS(getNum,"ALARM DEACTIVATED");
          
        }
      
    }
    
  }
  
}
// SMS received UPDATE
if(Serial.available()){
    lcd.clear();
    lcd.print("SMS UPDATE   ");
    gsmData = Serial.readString();
    if(gsmData.indexOf("+CMT:")>-1){
      int gsmIndex = gsmData.indexOf("+CMT:");
      int gsmLength = gsmData.length();
      // get the number of sender 
      // send UPDATE to the sender
      getNum = gsmData.substring(gsmIndex+7,gsmIndex+20);
      sendSMS(getNum,"UPDATE:\nMQ-2 Sensor: "+String(analogRead(A0))+"\nTemperature: "+String(tempValue)+" degree Celcius");
      lcd.clear();
      lcd.print("UPDATE SENT");
      delay(1000);
    }   
  }
}

