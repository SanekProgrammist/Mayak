#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WebServer.h>

char ssid[] = "Test1";          // Имя вашей Wi-Fi сети (SSID)
char password[] = "12345678o9";  // Пароль от вашей Wi-Fi сети
char mqtt_server[] = "mqtt.msur.team"; // Адрес вашего MQTT-брокера
//const char* mqtt_server = "mqtt.msur.team; // Адрес вашего MQTT-брокера
const uint16_t mqtt_port = 1883;              // Порт MQTT-брокера
char mqtt_user[] = "user";      // Логин для доступа к MQTT-брокеру
char mqtt_pass[] = "password";      // Пароль для доступа к MQTT-брокеру

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme; // I2C

WebServer server(80); // Создаем объект WebServer на порту 80
String receivedData = "1111";

unsigned long delayTime;

WiFiClient wifiClient;
PubSubClient client(wifiClient);


void handleNotFound() {
  server.send(404, "text/plain", "404: Not Found");
}

void handleFormSubmit() {
  if (server.method() == HTTP_POST) {
    for (uint8_t i = 0; i < server.args(); i++) {
      if (server.argName(i) == "data1") {
        
        receivedData = server.arg(i);
        strncpy(mqtt_user, receivedData.c_str(), sizeof(mqtt_user) - 1); 
        mqtt_user[sizeof(mqtt_user) - 1] = '\0'; 
        //mqtt_user[] = receivedData;
        Serial.println(receivedData);
        break;
      }
      if (server.argName(i) == "data2") {
        receivedData = server.arg(i);
        strncpy(mqtt_pass, receivedData.c_str(), sizeof(mqtt_pass) - 1); 
        mqtt_pass[sizeof(mqtt_pass) - 1] = '\0'; 
        //mqtt_pass[] = receivedData;
        Serial.println(receivedData);
        break;
      }
    }
    server.send(200, "text/html", "<h1>Данные получены: " + receivedData + "</h1>");
  } else {
    server.send(405, "text/plain", "Method Not Allowed");
  }
}

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

  server.on("/", handleRoot);
  server.on("/submit", HTTP_POST, handleFormSubmit);
  server.onNotFound(handleNotFound);

  // Запуск веб-сервера
  server.begin();
  Serial.println("HTTP server started");

}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    delay(5000);

    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void handleRoot() {
  String html = R"=====(
<!DOCTYPE html>
<html lang="ru">
<head>
  <meta charset="UTF-8">
  <title>Ввод данных</title>
</head>
<body>
  <h1>Введите данные:</h1>
  <form action="/submit" method="POST">
    <label for="dataInput">Введите логин:</label><br>
    <input type="text" id="dataInput1" name="data1"><br>
    <input type="submit" value="Отправить">
    <label for="dataInput">Введите пароль:</label><br>
    <input type="text" id="dataInput2" name="data2"><br>
    <input type="submit" value="Отправить">
  </form>
</body>
</html>
)=====";

  server.send(200, "text/html", html);
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

        server.handleClient();

  // Проверка новых данных
  if (receivedData.length() > 0) {
    Serial.println("Received data: " + receivedData);
    receivedData = ""; // Очистка переменной после обработки
    }
}
}
