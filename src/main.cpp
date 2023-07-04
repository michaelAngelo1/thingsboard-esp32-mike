#include <Arduino.h>
#include <Wire.h>
#include <Ticker.h>
#include <WiFi.h>
#include "BH1750.h"
#include "DHTesp.h"

// Thingsboard
#include "ThingsBoard.h"
#define THINGSBOARD_ACCESS_TOKEN "18vkrelzucy5wrghu1lb"
#define THINGSBOARD_SERVER "demo.thingsboard.io"

#define UPDATE_DATA_INTERVAL 5000
const char* ssid = "Mike's Hotspot";
const char* password = "1123581321";

WiFiClient espClient;
ThingsBoard tb(espClient);

#define PIN_DHT 13 
#define PIN_SDA 22
#define PIN_SCL 21

// Instantiate sensor classes
DHTesp dht;
BH1750 bh;

void sendToThingsBoard() {
  float hum = dht.getHumidity();
  float temp = dht.getTemperature();
  float lux = bh.readLightLevel();
  if(dht.getStatus() == DHTesp::ERROR_NONE) {
    Serial.printf("Temperature: %.2f C, Humidity: %.2f %%, Light: %.2f\n", temp, hum, lux);
    if(tb.connected()) {
      tb.sendTelemetryFloat("temperature", temp);
      tb.sendTelemetryFloat("humidity", hum);
    }
  }
  else {
    Serial.printf("Light: %.2f lx\n", lux);
  }
  tb.sendTelemetryFloat("Light", lux);
}

void WifiConnect() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  Serial.print("System connected with IP address: ");
  Serial.println(WiFi.localIP());
  Serial.printf("RSSI: %d\n", WiFi.RSSI());
}

void setup() {
  Serial.begin(9600);
  dht.setup(PIN_DHT, DHTesp::DHT11);
  Wire.begin(PIN_SDA, PIN_SCL);
  bh.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23, &Wire);
  WifiConnect();

  if(!tb.connected()) {
    if(tb.connect(THINGSBOARD_SERVER, THINGSBOARD_ACCESS_TOKEN)) {
      Serial.println("Connected to ThingsBoard");
    }
    else {
      Serial.println("Error connected to ThingsBoard");
      delay(3000);
    }
  }
}

void loop() {
  if(millis() % 3000 == 0 && tb.connected()) {
    sendToThingsBoard();
  }
  tb.loop();
}