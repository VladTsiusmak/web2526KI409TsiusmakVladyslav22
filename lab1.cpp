#include <WiFi.h> 
#include <WebServer.h> 
#include <LittleFS.h> 

// Налаштування Wi-Fi 
const char* ssid = "miron_shtyrm"; 
const char* password = "miron2019"; 

WebServer server(80); 
// Функція для зчитування файлу з LittleFS та відправки клієнту 
void handleRoot() { 
  File file = LittleFS.open("/index.html", "r"); 
  if (!file) { 
    server.send(404, "text/plain", "File not found"); 
    return; 
  } 

  server.streamFile(file, "text/html"); 
  file.close(); 
} 

void setup() { 
  Serial.begin(115200); 
  // 1. Ініціалізація LittleFS 
  if (!LittleFS.begin(true)) { 
    Serial.println("Помилка LittleFS!"); 
    return; 
  } 

  // 2. Підключення до Wi-Fi 
  WiFi.begin(ssid, password); 
  Serial.print("Підключення до Wi-Fi"); 
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); 
    Serial.print("."); 
  } 

  Serial.println("\nWi-Fi підключено!"); 
  Serial.print("IP адреса: "); 
  Serial.println(WiFi.localIP()); 

  // 3. Налаштування маршрутів сервера 
  server.on("/", HTTP_GET, handleRoot); 

  // 4. Запуск сервера 
  server.begin(); 
  Serial.println("HTTP сервер запущено"); 
} 

void loop() { 
  server.handleClient(); 
} 
