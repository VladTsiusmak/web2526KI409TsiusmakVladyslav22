#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
#include <LittleFS.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <ArduinoJson.h>

// BME280
Adafruit_BME280 bme;

// Wi-Fi налаштування
const char* ssid = "miron_shtyrm";
const char* password = "miron2019";

// Сервери
AsyncWebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

// Обробник WebSocket
void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

    switch(type) {

        case WStype_DISCONNECTED:
            Serial.printf("[%u] Клієнт відключився\n", num);
            break;

        case WStype_CONNECTED:
            Serial.printf("[%u] Клієнт підключився з IP: %s\n",
            num, webSocket.remoteIP(num).toString().c_str());
            break;

        case WStype_TEXT:
            Serial.printf("[%u] Отримано повідомлення: %s\n", num, payload);
            break;

        default:
            break;
    }
}

void setup() {

    Serial.begin(115200);
    delay(1000);

    // I2C запуск
    Wire.begin();

    // Ініціалізація BME280
    if (!bme.begin(0x76)) {
        Serial.println("Помилка: BME280 не знайдено!");
        while (1);
    }

    // LittleFS
    if(!LittleFS.begin(true)) {
        Serial.println("Помилка LittleFS!");
        return;
    }

    // Підключення WiFi
    Serial.printf("Підключення до %s ", ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWi-Fi підключено!");
    Serial.print("IP адреса ESP32: ");
    Serial.println(WiFi.localIP());

    // HTML сторінка
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/index.html", "text/html");
    });

    server.begin();

    // WebSocket
    webSocket.begin();
    webSocket.onEvent(onWebSocketEvent);
}

void loop() {

    webSocket.loop();

    static unsigned long lastUpdate = 0;

    if (millis() - lastUpdate >= 2000) {

        lastUpdate = millis();

        float humidity = bme.readHumidity();
        float pressure = bme.readPressure() / 100.0F; // hPa

        if (isnan(humidity) || isnan(pressure)) {
            Serial.println("Помилка зчитування BME280!");
            return;
        }

        // JSON
        JsonDocument doc;

        doc["pressure"] = pressure;
        doc["humidity"] = humidity;

        String jsonString;
        serializeJson(doc, jsonString);

        // Відправка всім клієнтам
        webSocket.broadcastTXT(jsonString);

        Serial.print("Відправлено дані: ");
        Serial.println(jsonString);
    }
}
