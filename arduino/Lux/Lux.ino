#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

//ACCESS POINT

const char* toSSID = "Replace to your SSID";
const char* key = "Replace to your PSK";
WiFiClientSecure client;

// Setting for Kintone
const char* HOST = "xxx.cybozu.com"; // Replace to your Kintone's Hostname
const int   PORT = 443;
const int   APPID = 10; // Replace to your Kintone's Application ID
const char* PATH = "/k/v1/record.json";
const char* APITOKEN = "??????"; // Replace to your Application's API Token
// *.cybozu.com's Fingerprint (2016/05/27)
//const char* fingerprint = "5C 06 F4 75 33 80 9D 7D FA CE 26 7D BD 05 63 D0 54 20 6A C3";

// Sensor Setting/Library Include
#include <Wire.h>
#include "Adafruit_TSL2591.h"
const uint16_t MAX_LUX = 88000;

// connect SCL to analog 5
// connect SDA to analog 4
// connect Vin to 3.3-5V DC
// connect GROUND to common ground

Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591); // pass in a number for the sensor identifier (for your use later)

void configureSensor(void)
{
  // You can change the gain on the fly, to adapt to brighter/dimmer light situations
  //tsl.setGain(TSL2591_GAIN_LOW);    // 1x gain (bright light)
  tsl.setGain(TSL2591_GAIN_MED);      // 25x gain
  // tsl.setGain(TSL2591_GAIN_HIGH);   // 428x gain

  // Changing the integration time gives you a longer time over which to sense light
  // longer timelines are slower, but are good in very low light situtations!
  tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);  // shortest integration time (bright light)
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_200MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_400MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_500MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_600MS);  // longest integration time (dim light)

  /* Display the gain and integration time for reference sake */
  Serial.println("------------------------------------");
  Serial.print  ("Gain:         ");
  tsl2591Gain_t gain = tsl.getGain();
  switch(gain)
  {
    case TSL2591_GAIN_LOW:
      Serial.println("1x (Low)");
      break;
    case TSL2591_GAIN_MED:
      Serial.println("25x (Medium)");
      break;
    case TSL2591_GAIN_HIGH:
      Serial.println("428x (High)");
      break;
    case TSL2591_GAIN_MAX:
      Serial.println("9876x (Max)");
      break;
  }
  Serial.print  ("Timing:       ");
  Serial.print((tsl.getTiming() + 1) * 100, DEC);
  Serial.println(" ms");
  Serial.println("------------------------------------");
  Serial.println("");
}

uint16_t getLux(void)
{
  bool flg = false;
  uint16_t current = 0;
  while(!flg){
    uint32_t lum = tsl.getFullLuminosity();
    uint16_t ir, full;
    ir = lum >> 16;
    full = lum & 0xFFFF;
    current = tsl.calculateLux(full, ir);
    if(current <= MAX_LUX){
      flg = true;
    }
  }
  return current;
}

void WiFiConnect(){
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.printDiag(Serial);
  WiFi.begin(toSSID,key);
  while (WiFi.status() != WL_CONNECTED){
    WiFi.printDiag(Serial);
    delay(500);
  }

  Serial.println();
  Serial.println("WiFi Connected");
}



void postLux(uint16_t lux) {

  while (!client.connect(HOST,PORT)) {
    Serial.println("Connection Faild...");
  }
  /*
  if(client.verify(fingerprint,host)) {
    Serial.println("Certificate matchs");
  } else {
    Serial.println("Certificate dosen't matchs");
  }
  */
  char data[] = "";
  sprintf(data,"{app: %i, record: { lux: { value: %lu } }}\n",APPID,lux);
  Serial.print(data);
  //Serial.print(strlen(data));
  client.print("POST ");client.print(PATH);client.print(" HTTP/1.1\n");
  client.print("Host: ");client.print(HOST);client.print(":443\n");
  client.print("X-Cybozu-API-Token: ");client.print(APITOKEN);client.print("\n");
  client.print("Conection: close\n");
  client.print("Content-type: application/json\n");
  client.print("Content-Length: ");client.print(strlen(data) - 1);client.print("\n");
  client.print("\n");
  client.print(data);

  /*
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  */
  //Serial.println("Request Done!");
}

void setup() {
  Serial.println("Starting Adafruit TSL2591 Test!");
  if (tsl.begin()) {
    Serial.println("Found a TSL2591 sensor");
  } else {
    Serial.println("No sensor found ... check your wiring?");
    while (1);
  }
  /* Configure the sensor */
  configureSensor();
  // put your setup code here, to run once:
  WiFiConnect();
}

void loop() {
  // put your main code here, to run repeatedly:
  postLux(getLux());
  delay(60000);
}
