#include <Arduino.h> 

#include <WiFi.h> 

#include <ESPAsyncWebServer.h> 

#include <WebSocketsServer.h> 

#include <LittleFS.h> 

#include <DHT.h> 

#include <ArduinoJson.h> 

 

// Налаштування DHT11 

#define DHTPIN 4        // DATA пін підключи до D4 

#define DHTTYPE DHT11 

DHT dht(DHTPIN, DHTTYPE); 

 

// Налаштування Wi-Fi - ВПИШИ СВОЇ ДАНІ 

const char* ssid = "miron_shtyrm"; 

const char* password = "miron2019"; 

 

AsyncWebServer server(80); 

WebSocketsServer webSocket = WebSocketsServer(81); 

 

// Виправлена функція обробки подій WebSocket 

void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) { 

    switch(type) { 

        case WStype_DISCONNECTED: 

            Serial.printf("[%u] Клієнт відключився\n", num); 

            break; 

        case WStype_CONNECTED: 

            Serial.printf("[%u] Клієнт підключився з IP: %s\n", num, webSocket.remoteIP(num).toString().c_str()); 

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

     

    dht.begin(); 

 

    // Ініціалізація LittleFS (true - форматувати, якщо не завантажено образ) 

    if(!LittleFS.begin(true)) { 

        Serial.println("Помилка LittleFS! Перевірте завантаження Filesystem Image."); 

        return; 

    } 

 

    // Підключення до Wi-Fi 

    Serial.printf("Підключення до %s ", ssid); 

    WiFi.begin(ssid, password); 

    while (WiFi.status() != WL_CONNECTED) { 

        delay(500); 

        Serial.print("."); 

    } 

    Serial.println("\nWi-Fi підключено!"); 

    Serial.print("IP адреса ESP32: "); 

    Serial.println(WiFi.localIP()); 

 

    // Налаштування маршруту для HTML сторінки 

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){ 

        request->send(LittleFS, "/index.html", "text/html"); 

    }); 

 

    server.begin(); 

     

    // Налаштування WebSocket 

    webSocket.begin(); 

    webSocket.onEvent(onWebSocketEvent); 

} 

 

void loop() { 

    webSocket.loop(); 

 

    // Читаємо датчик кожні 2 секунди 

    static unsigned long lastUpdate = 0; 

    if (millis() - lastUpdate >= 2000) { 

        lastUpdate = millis(); 

 

        float h = dht.readHumidity(); 

        float t = dht.readTemperature(); 

 

        // Перевірка на помилки зчитування 

        if (isnan(h) || isnan(t)) { 

            Serial.println("Помилка зчитування з DHT11!"); 

            return; 

        } 

 

        // Створення JSON об'єкта 

        JsonDocument doc; 

        doc["temp"] = t; 

        doc["hum"] = h; 

 

        String jsonString; 

        serializeJson(doc, jsonString); 

 

        // Відправка всім підключеним клієнтам 

        webSocket.broadcastTXT(jsonString); 

         

        Serial.print("Відправлено дані: "); 

        Serial.println(jsonString); 

    } 

} 
