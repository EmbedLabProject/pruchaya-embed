#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecureBearSSL.h>
EspSoftwareSerial::UART testSerial;


const char* ssid = "MARK_HOMELAND_2.4G";
const char* password = "markthitrin";

const String backendurl = "http://192.168.1.102:3222";
const String deviceId = "69420";

unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

String curLightValue = "0";
String curHumidValue = "0";
String curVibrateValue = "0";
char buffer[256];
int buffer_i = 0;

void get_sensor() {
  while (testSerial.available() > 0) {
    // recieve and echo back for acknowledge
    char input = testSerial.read();
    testSerial.print(input);

    buffer[buffer_i++] = input;
    if(buffer[buffer_i - 1] != '\n') {
      continue;
    }

    String data[4];
    int data_i = 0;
    for(int i = 0 ;i < buffer_i;i++) {
      if(isDigit(buffer[i])) {
        data[data_i] += buffer[i];
      }
      else{
        data_i++;
      }
    }
    curLightValue = data[0];
    curHumidValue = data[1];
    curVibrateValue = data[2];
    buffer_i = 0;
    data_i = 0;
    break;
  }
}

void send_data() {
  if((millis() - lastTime) > timerDelay) {
    
    if(WiFi.status() == WL_CONNECTED){
      lastTime = millis();
      std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
      // Ignore SSL certificate validation
      client->setInsecure();

      WiFiClient clientNonSecure;

      //create an HTTPClient instance
      HTTPClient https;

      String serverPath = backendurl + "/sensors/update";
      //
      //
      
      https.begin(clientNonSecure, serverPath);
      https.addHeader("Content-Type", "application/json");
      https.addHeader("ngrok-skip-browser-warning", "69420");

      String body = "{\"device_id\":\"" + deviceId + "\",\"light\":\"" + curLightValue + "\",\"humidity\":\""+curHumidValue+"\",\"vibration\":\""+curVibrateValue+"\"}";
      Serial.println(body);

      int httpResponseCode = https.POST(body);
      Serial.print("String: ");
      Serial.println(https.getString());
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      if(httpResponseCode < 0) {
        Serial.println(https.errorToString(httpResponseCode).c_str());
      }
      https.end();
      Serial.println("sent");
      Serial.println(curLightValue + " " + curHumidValue + " " + curVibrateValue);
    }
    else {
      Serial.println("Wifi disconnected");
    }
  }
}

void setup() {
  testSerial.begin(115200, EspSoftwareSerial::SWSERIAL_8N1, D7, D8, false, 95, 11);
  Serial.begin(9600);

  // connecting to WIFI
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}
void loop() {
  get_sensor();
  send_data();
}
