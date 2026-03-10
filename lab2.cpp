#include <Arduino.h> 

#include <WiFi.h> 

#include <WebServer.h> 

#include <LittleFS.h> 

#include <ArduinoJson.h> 

 

const char* ssid = "miron_shtyrm"; 

const char* password = "miron2019"; 

 

WebServer server(80); 

const int LED_PIN = 2;

bool ledState = false; 

 

void handleGetStatus() { 

    JsonDocument doc; 

    doc["led_on"] = ledState; 

    String response; 

    serializeJson(doc, response); 

    server.send(200, "application/json", response); 

} 

 

void handlePostControl() { 

    if (server.hasArg("plain")) { 

        String body = server.arg("plain"); 

        JsonDocument doc; 

        DeserializationError error = deserializeJson(doc, body); 

 

        if (!error) { 

            String command = doc["command"]; 

            if (command == "on") ledState = true; 

            else if (command == "off") ledState = false; 

             

            digitalWrite(LED_PIN, ledState ? HIGH : LOW); 

            server.send(200, "application/json", "{\"result\":\"ok\"}"); 

        } else { 

            server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}"); 

        } 

    } else { 

        server.send(400, "text/plain", "Body missing"); 

    } 

} 

 

void setup() { 

    Serial.begin(115200); 

    pinMode(LED_PIN, OUTPUT); 

    digitalWrite(LED_PIN, LOW); 



    if(!LittleFS.begin(true)) { 

        Serial.println("Помилка монтування LittleFS!"); 

        return; 

    } 

    Serial.println("LittleFS змонтовано."); 

 

    WiFi.begin(ssid, password); 

    Serial.print("Підключення до WiFi"); 

    while (WiFi.status() != WL_CONNECTED) { 

        delay(500); 

        Serial.print("."); 

    } 

    Serial.println("\nWiFi підключено!"); 

    Serial.print("IP адреса: "); 

    Serial.println(WiFi.localIP()); 

 


    server.on("/", HTTP_GET, []() { 

        if (LittleFS.exists("/index.html")) { 

            File file = LittleFS.open("/index.html", "r"); 

            server.streamFile(file, "text/html"); 

            file.close(); 

        } else { 

            server.send(404, "text/plain", "Файл index.html не знайдено в LittleFS. Завантажте папку data через Upload Filesystem Image."); 

        } 

    }); 

 

    server.on("/api/status", HTTP_GET, handleGetStatus); 

    server.on("/api/control", HTTP_POST, handlePostControl); 

 

    server.begin(); 

    Serial.println("HTTP сервер запущено."); 

} 

 

void loop() { 

    server.handleClient(); 

} 
