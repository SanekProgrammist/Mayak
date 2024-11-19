#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "ArduinoJson.h"
 
 
#define SEALEVELPRESSURE_HPA (1013.25)
 
Adafruit_BME280 bme; // I2C
 
unsigned long delayTime;
 
void setup() {
  Serial.begin(9600);
  Serial.println(F("BME280 test"));
 
  bool status;
 
  // настройки по умолчанию
  status = bme.begin(0x76);  
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
 
  Serial.println("-- Default Test --");
  delayTime = 1000;
 
  Serial.println();
}
 
 
void loop() { 
  
  delay(delayTime);
  JsonDocument JSONencoder;

  JSONencoder["team"] = "Команда 2";
  JSONencoder["temperature"] = bme.readTemperature();
  JSONencoder["humidity"] = bme.readHumidity();
  JSONencoder["pressure"] = (bme.readPressure() / 100.0F);
  

  serializeJson(JSONencoder, Serial);

  Serial.println();

  delay(5000);
}
 
