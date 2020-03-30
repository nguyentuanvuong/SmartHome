#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <SimpleTimer.h>
#include <ESP8266WebServer.h>
#include "EEPROM.h"
#include "index.h"

ESP8266WebServer server(80);
String myString, cdata;
char rdata;
int btn1Status, btn2Status, relay1Status, relay2Status;
int temp, humi, gas;

SimpleTimer timer;

void setup() {
  Serial.begin(9600);
  EEPROM.begin(512);
  String essid;
  for (int i = 0; i < 32; ++i) {
    essid += char(EEPROM.read(i));
  }
  Serial.print("SSID: ");
  Serial.println(essid.c_str());
  essid.trim();
  String epass = "";
  for (int i = 32; i < 96; ++i) {
    epass += char(EEPROM.read(i));
  }
  Serial.print("PASS: ");
  Serial.println(epass.c_str());
  essid.trim();
  String etoken = "";
  for (int i = 96; i < 128; ++i) {
    etoken += char(EEPROM.read(i));
  }
  Serial.print("BLYNK: ");
  Serial.println(etoken.c_str());
  etoken.trim();
  WiFi.begin(essid.c_str(), epass.c_str());
  if (testWifi()) {
    WiFi.disconnect();
    Blynk.begin(etoken.c_str(), essid.c_str(), epass.c_str(), IPAddress(14,236,12,124), 8080);
    //  Blynk.begin(auth, ssid, pass, "nguyentuanvuong.tk", 9443);
    
    timer.setInterval(1, btnStatus);
    timer.setInterval(1, sensorValue);
    return;
  } else {
    SetupAP();
    WebServer();
  }
}

void WebServer() {
  while (true) {
    server.handleClient();
  }
}

void SetupAP(void) {
  WiFi.mode(WIFI_STA);
  WiFi.softAP("Smart Home", "");
  WiFi.disconnect();
  delay(100);
  Serial.println("WiFi ket noi:\nSmart Home");
  Serial.print("Dia chi IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());
  createWebServer();
  server.begin();
}

void createWebServer() {
  server.on("/", [] {
    server.send(200, "text/html", MAIN_page);
  });
  server.on("/action", [] {
    String qssid = server.arg("ssid");
    String qpass = server.arg("pass");
    String qtoken = server.arg("token");
    if (qssid.length() > 0 && qpass.length() > 0) {
      EEPROM.begin(512);
      Serial.println("Xóa Bộ nhớ");
      for (int i = 0; i < 128; ++i) {
        EEPROM.write(i, 0);
      }
      EEPROM.commit();
      for (int i = 0; i < qssid.length(); ++i) {
        EEPROM.write(i, qssid[i]);
      }
      for (int i = 0; i < qpass.length(); ++i) {
        EEPROM.write(32 + i, qpass[i]);
      }
      for (int i = 0; i < qtoken.length(); ++i) {
        EEPROM.write(96 + i, qtoken[i]);
      }
      EEPROM.commit();
      EEPROM.end();
    }
    Serial.println("Tên Wifi" + qssid + "\nMật khẩu" + qpass + "\nToken Blynk " + qtoken+"\n Đã lưu vào hệ thống\nKhởi ộng lại để hoàn tất thiết lập");
    server.send(200, "text/html", "Khoi dong lại de hoan tat thiet lap");
  });
}

bool testWifi(void) {
  int c = 0;
  Serial.println("Chờ kết nối wifi ......");
  while (c < 20) {
    if (WiFi.status() == WL_CONNECTED) {
      return true;
    }
    delay(1000);
    Serial.print(".");
    c++;
  }
  Serial.println("\nThời gian kết nối chậm, Mở AP");
  return false;
}

void loop() {
  if (Serial.available() == 0) {
    Blynk.run();
    timer.run();
  }
  if (Serial.available() > 0) {
    rdata = Serial.read();

    myString = myString + rdata;
    if (rdata == '\n') {
      String key = getValue(myString, ',', 0);

      if (key == "relay1") {
        String btn1 = getValue(myString, ',', 1);
        btn1Status = btn1.toInt();
      }
      if (key == "relay2") {
        String btn2 = getValue(myString, ',', 1);
        btn2Status = btn2.toInt();
      }
      if (key == "sensor") {
        String t = getValue(myString, ',', 1);
        String h = getValue(myString, ',', 2);
        String g = getValue(myString, ',', 3);
        temp = t.toInt();
        humi = h.toInt();
        gas = g.toInt();
        if(gas>500){
          Blynk.notify("Cảnh báo gas rò rỉ");
        }
      }
      myString = "";
    }
  }

}

String getValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = {
    0,
    -1
  };
  int maxIndex = data.length() - 1;
  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

BLYNK_WRITE(V4) {
  int pinValue = param.asInt();
  if (pinValue == 0) {
    relay1Status = 0;
  } else {
    relay1Status = 1;
  }
  cdata = cdata + "V4," + relay1Status;
  Serial.println(cdata);
  cdata = "";
}

BLYNK_WRITE(V5) {
  int pinValue = param.asInt();
  if (pinValue == 0) {
    relay2Status = 0;
  } else {
    relay2Status = 1;
  }
  cdata = cdata + "V5," + relay2Status;
  Serial.println(cdata);
  cdata = "";
}

void btnStatus() {
  //  int sdata = btn1Status;
  Blynk.virtualWrite(V4, btn1Status);
  Blynk.virtualWrite(V5, btn2Status);
}

void sensorValue() {
  Blynk.virtualWrite(V1, temp);
  Blynk.virtualWrite(V2, humi);
  Blynk.virtualWrite(V3, gas);
}
