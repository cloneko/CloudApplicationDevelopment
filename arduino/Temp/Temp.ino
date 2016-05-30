#include "floatToString.h"

// MCP9808(Temprerature sensor)
#include <Wire.h>
#include "Adafruit_MCP9808.h"

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


// Create the MCP9808 temperature sensor object
Adafruit_MCP9808 tempsensor = Adafruit_MCP9808();

float getTemperature (){
  tempsensor.shutdown_wake(0);
  float c = tempsensor.readTempC();
  delay(250);
  tempsensor.shutdown_wake(1);
  return c;
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



void postTemp(float temp) {

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
  char str_temp[60] = "";
  floatToString(str_temp,temp,2);
  sprintf(data,"{app: %i, record: { temp: { value: %s } }}\n",APPID,str_temp);
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
  Serial.println("Request Done!");
}

void setup() {
  Serial.begin(115200);
  // Make sure the sensor is found, you can also pass in a different i2c
  // address with tempsensor.begin(0x19) for example
  if (!tempsensor.begin()) {
    Serial.println("Couldn't find MCP9808!");
    while (1);
  }
  WiFiConnect();
}

void loop() {
  postTemp(getTemperature());
  delay(60000 * 5); // 5min
}
