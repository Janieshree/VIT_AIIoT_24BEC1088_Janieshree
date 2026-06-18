#include <WiFi.h>
#include "ThingSpeak.h"
#include "DHTesp.h"
#include "RTClib.h" 

const int DHT_PIN = 15; 
const int LED_PIN = 18;

DHTesp dhtSensor;
RTC_DS1307 rtc; 
WiFiClient client;

const char* WIFI_NAME = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";
const int myChannelNumber = 3411377; //Channel ID
const char* myApiKey = "HLS3WAQFEFY7TJF7"; //Write API Key

unsigned long lastThingSpeakUpdate = 0;
const unsigned long thingSpeakInterval = 15000; // Updates ThingSpeak data for every 15 seconds

void setup() {
  Serial.begin(115200);
  delay(1000); 
  Serial.println("System Starting");

  pinMode(LED_PIN, OUTPUT);
  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);
//warning message in case of data not received from RTC module
  if (!rtc.begin()) {
    Serial.println("!! RTC missing! Check I2C wiring.");
    while (1);
  }

  Serial.print("~Connecting to WiFi");
  WiFi.begin(WIFI_NAME, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n WiFi Connected!");

  ThingSpeak.begin(client);
  
  //Waiting 5 seconds for network gateway connection stabilization
  delay(5000); 
  Serial.println("System Ready");//System is ready to receive data from DHT22
}

void loop() {
  TempAndHumidity data = dhtSensor.getTempAndHumidity(); //Read Temperature and humidity from DHT22
  DateTime now = rtc.now(); //get time from RTC module

  // Print Time and Temperature to Serial Monitor (Updates every second)
  Serial.print("[");
  if(now.hour() < 10) Serial.print('0'); Serial.print(now.hour()); Serial.print(':');
  if(now.minute() < 10) Serial.print('0'); Serial.print(now.minute()); Serial.print(':');
  if(now.second() < 10) Serial.print('0'); Serial.print(now.second());
  Serial.print("] Temp: ");
  Serial.print(data.temperature, 1);
  Serial.println(" C");

  // Check logic 
  if (data.temperature > 40.0) {
    digitalWrite(LED_PIN, HIGH);//LED indicator when temp. exceeded
    Serial.println("Warning: Limit Exceeded!");//alert message
  } else {
    digitalWrite(LED_PIN, LOW);//else LED stays off
  }

  // Send data to Cloud (for every 15 seconds)
  if (millis() - lastThingSpeakUpdate >= thingSpeakInterval) {
    Serial.println("Uploading to ThingSpeak...");
    ThingSpeak.setField(1, data.temperature);
    int responseCode = ThingSpeak.writeFields(myChannelNumber, myApiKey);
    // to ensure data gets uploaded(mainly for debugging purpose)
    if(responseCode == 200) {
      Serial.println("ThingSpeak updated successfully!");
    } else {
      Serial.print("<>ThingSpeak failed. HTTP Code: ");
      Serial.println(responseCode);
    }
    
    lastThingSpeakUpdate = millis();
  }

  delay(1000); // Check and print values once every second
}
