#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <SimpleTimer.h>
#include <IRremote.h>
#include <Servo.h>
#include <DHT.h>

Servo myservo;

SoftwareSerial esp8266(3,2);//tx-2, rx-3
LiquidCrystal_I2C lcd(0x27,16,2);
SimpleTimer timer;

int btn1 = 4;
int btn2 = 5;

int relay1 = 6;
int relay2 = 7;

const int DHTPIN = 8;
const int DHTTYPE = DHT11;
DHT dht(DHTPIN, DHTTYPE);

int baodong = 9;

const int receiverPin = 10;

IRrecv irrecv(receiverPin);
decode_results results;

int SensorGas = A0;
int bientro = A1;
int servoPin = 11;

String cdata,myString;
char rdata;

byte relay1Status = 0;
byte relay2Status = 0;

void setup() {
  lcd.begin();  
  lcd.backlight();
  lcd.clear();
  
  lcd.setCursor(0,0);
  lcd.print("T:");
  lcd.setCursor(0,1);
  lcd.print("H:");
  lcd.setCursor(7,0);
  lcd.print("Gas:");
  
	Serial.begin(9600); 
	esp8266.begin(9600);

  pinMode(btn1, INPUT);
  pinMode(btn2, INPUT);

  pinMode(SensorGas,INPUT);

  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(baodong, OUTPUT);

  myservo.attach(servoPin);

  irrecv.enableIRIn();

  digitalWrite(baodong, 0);
    
  timer.setInterval(0, Button_1);
  timer.setInterval(0, Button_2);
  timer.setInterval(3000, Sensor);
  timer.setInterval(0, remote);

}

void loop() {
  timer.run();

  if(Serial.available() > 0){
    rdata = Serial.read();
    
    myString = myString + rdata;
    if(rdata == '\n'){
      String key = getValue(myString, ',', 0);
      
      if(key == "V4"){
        String relay1 = getValue(myString, ',',1);
        relay1Status = relay1.toInt();
        cdata = cdata+"relay1,"+relay1Status;
        Serial.println(cdata);
        cdata ="";
      }
      if(key == "V5"){
        String relay2 = getValue(myString, ',',1);
        relay2Status = relay2.toInt();
        cdata = cdata+"relay2,"+relay2Status;
        Serial.println(cdata);
        cdata ="";
      }
      myString = "";
    }
  }

    digitalWrite(relay1, relay1Status);
    digitalWrite(relay2, relay2Status);
}

void Button_1(){
  if(digitalRead(btn1) == 1){
    int i = 0;
    while (digitalRead(btn1) == 1);
      relay1Status = !relay1Status;
      cdata = cdata+"relay1,"+relay1Status;
      Serial.println(cdata);
      cdata ="";
  }
}
void Button_2(){
  if(digitalRead(btn2) == 1){
    while (digitalRead(btn2) == 1);
    relay2Status = !relay2Status;
    cdata = cdata+"relay2,"+relay2Status;
    Serial.println(cdata);
    cdata ="";
  }
}

void Sensor(){
  int h = dht.readHumidity();// Đoc nhiệt độ
  int t = dht.readTemperature();// Đọc độ ẩm
  int g = analogRead(SensorGas);// Đọc nồng độ khí Gas
  if(g > 500){
    digitalWrite(baodong, 1);
  }
  else{
    digitalWrite(baodong, 0);
  }
  if(isnan(t) || isnan(h)){
    
  }
  else{
    lcd.setCursor(3,0);
    lcd.print(t);
    lcd.setCursor(3,1);
    lcd.print(h);
    lcd.setCursor(11,0);
    lcd.print(g);

    cdata = cdata+"sensor,"+t+","+h+","+g;
    Serial.println(cdata);
    cdata ="";
  }
  
}

void remote(){
  if(irrecv.decode(&results)){
    if(results.value == 16724175){ // Phím 1
      relay1Status = !relay1Status;
      cdata = cdata+"relay1,"+relay1Status;
      Serial.println(cdata);
      cdata ="";
    }
    if(results.value == 16718055){ // Phím 2
      relay2Status = !relay2Status;
      cdata = cdata+"relay2,"+relay2Status;
      Serial.println(cdata);
      cdata ="";
    }
    if(results.value == 16753245 ){ // Phím CH-
      int value = 0;
      int servoPos = map(value, 0, 1023, 0, 180);
      myservo.write(servoPos);
    }
    if(results.value == 16769565 ){ // Phím CH+
      int value = 1023;
      int servoPos = map(value, 0, 1023, 0, 180);
      myservo.write(servoPos);
    }
    
    irrecv.resume();
  }
}

String getValue(String data, char separator, int index){
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;
  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i ) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
