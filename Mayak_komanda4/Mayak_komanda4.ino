#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <PubSubClient.h>
//#include <WebServer.h>

char ssid[] = "Test1";          // Имя вашей Wi-Fi сети (SSID)
char password[] = "12345678o9";  // Пароль от вашей Wi-Fi сети
char mqtt_server[] = "mqtt.msur.team"; // Адрес вашего MQTT-брокера
//const char* mqtt_server = "mqtt.msur.team; // Адрес вашего MQTT-брокера
const uint16_t mqtt_port = 1883;              // Порт MQTT-брокера
char mqtt_user[] = "msur_user";      // Логин для доступа к MQTT-брокеру
char mqtt_pass[] = "msur_password";      // Пароль для доступа к MQTT-брокеру

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme; // I2C

unsigned long delayTime;

WiFiClient wifiClient;
PubSubClient client(wifiClient);

void setup() {
    Serial.begin(115200);

    bool status = bme.begin(0x76); // Или другой адрес BME280, если отличается
    if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        while (1);
    }

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    client.setServer(mqtt_server, mqtt_port);
}

void reconnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");

        String clientId = "ESP32Client-";
        clientId += String(random(0xffff), HEX);

        if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
            Serial.println("connected");
            client.publish("esp32/outTopic", "hello world");
            client.subscribe("esp32/inTopic");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    static unsigned long lastMsgTime = 0;
    if (millis() - lastMsgTime > 5000) {
        lastMsgTime = millis();

        JsonDocument doc;
        doc["team"] = "Команда 2";
        doc["temperature"] = bme.readTemperature();
        doc["humidity"] = bme.readHumidity();
        doc["pressure"] = bme.readPressure() / 100.0F;

        char jsonBuffer[256];
        serializeJson(doc, jsonBuffer);

        Serial.print("Sending message: ");
        Serial.println(jsonBuffer);

        client.publish("esp32/topic", jsonBuffer);
    }
}
 
