#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecureBearSSL.h>

EspSoftwareSerial::UART UART_STM32;

const char* ssid = "Mark";
const char* password = "1234567ww";

const String backendUrl = "https://pruchaya-deploy-backend.onrender.com";
const String deviceId = "69420";

unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

unsigned long lastTimeSensor = 0;
unsigned long sensorFetchTimeout = 0;  

char buffer[256];
int bufferI = 0;
String curLightValue = "0";
String curHumidValue = "0";
String curVibrateValue = "0";

void get_sensor() {
  while (UART_STM32.available() > 0) {
    // Recieve and echo back for acknowledge
    char input = UART_STM32.read();
    UART_STM32.print(input);

    // Put character into buffer until getting '\n'
    buffer[bufferI++] = input;
    if(bufferI >= 256) { 
      bufferI = 0;
    }
    if(input != '\n') {
      continue;
    }

    // Transfer string in buffer
    String data[4];
    int dataI = 0;
    for(int i = 0 ;i < bufferI;i++) {
      if(isDigit(buffer[i])) {
        data[dataI] += buffer[i];
      }
      else{
        dataI++;
      }

      if(dataI >= 4){
        break;
      }
    }

    curLightValue = data[0];
    if(curLightValue.length() == 0) {
      curLightValue = "0";
    }
    curHumidValue = data[1];
    if(curHumidValue.length() == 0) {
      curHumidValue = "0";
    }
    curVibrateValue = data[2];
    if(curVibrateValue.length() == 0) {
      curVibrateValue = "0";
    }

    bufferI = 0;
    dataI = 0;
    break;
  }
}

void send_data() {
  if((millis() - lastTime) > timerDelay) {
    
    if(WiFi.status() == WL_CONNECTED){
      // Create WIFI client
      // Use httpClient(*client,severPath) if the server rquire https(http with "s")
      std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
      client->setInsecure();
      // Use httpClient(clientNonSecure,severPath) for http server
      WiFiClient clientNonSecure;

      // Create an HTTPClient instance and send post request.
      HTTPClient httpClient;
      httpClient.setTimeout(5000);
      String serverPath = backendUrl + "/sensors/update";
      httpClient.begin(*client, "https://pruchaya-deploy-backend.onrender.com/sensors/update");
      httpClient.addHeader("Content-Type", "application/json");
      String body = "{\"device_id\":\"" + deviceId + 
                    "\",\"light\":\"" + curLightValue + 
                    "\",\"humidity\":\"" + curHumidValue + 
                    "\",\"vibration\":\"" + curVibrateValue + "\"}";
      int httpResponseCode = httpClient.POST(body);

      // check respone code and print serial
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      if(httpResponseCode < 0) {
        Serial.println("(" + httpClient.errorToString(httpResponseCode) + ")");
      }

      // End request
      httpClient.end();

      Serial.print("Value sent : ");
      Serial.println("Light : " + curLightValue + ",Humidity : " + curHumidValue + ",Vibration : " + curVibrateValue);

      lastTime = millis();
    }
    else {
      Serial.println("Wifi disconnected");
    }
  }
}

void setup() {
  // Init UART connecting to STM32
  UART_STM32.begin(115200, EspSoftwareSerial::SWSERIAL_8N1, D7, D8, false, 95, 11);
  // Init Serial for debuging
  Serial.begin(9600);
  // Init WIFI
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
  delay(100);
}
