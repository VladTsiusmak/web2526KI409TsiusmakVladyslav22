#include <Arduino.h> 

#include <WiFi.h> 

#include <DNSServer.h> 

#include <WebServer.h> 

#include <Preferences.h> 

#include <LittleFS.h> 

 

WebServer server(80); 

DNSServer dnsServer; 

Preferences preferences; 

 

const byte DNS_PORT = 53; 

IPAddress apIP(192, 168, 1, 1); 

 

void handleSave() { 

    String new_ssid = server.arg("ssid"); 

    String new_pass = server.arg("pass"); 

 

    if (new_ssid.length() > 0) { 

        preferences.begin("wifi-creds", false); 

        preferences.putString("ssid", new_ssid); 

        preferences.putString("pass", new_pass); 

        preferences.end(); 

 

        server.send(200, "text/html", "<h2> Збережено! ESP32 перезавантажується...</h2>"); 

        delay(2000); 

        ESP.restart(); 

    } 

} 

 

void setup() { 

    Serial.begin(115200); 

     

    // Ініціалізація LittleFS 

    if(!LittleFS.begin(true)) { 

        Serial.println("LittleFS Error!"); 

        return; 

    } 

 

    preferences.begin("wifi-creds", true); 

    String saved_ssid = preferences.getString("ssid", ""); 

    String saved_pass = preferences.getString("pass", ""); 

    preferences.end(); 

 

    if (saved_ssid != "") { 

        WiFi.begin(saved_ssid.c_str(), saved_pass.c_str()); 

        Serial.print("Connecting to: "); Serial.println(saved_ssid); 

         

        int counter = 0; 

        while (WiFi.status() != WL_CONNECTED && counter < 20) { 

            delay(500); Serial.print("."); counter++; 

        } 

    } 

 

    if (WiFi.status() != WL_CONNECTED) { 

        WiFi.mode(WIFI_AP); 

        WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0)); 

        WiFi.softAP("Tsiusmak_Setup"); 

 

        dnsServer.start(DNS_PORT, "*", apIP); 

 

        // Головна сторінка Captive Portal з файлу 

        server.on("/", HTTP_GET, []() { 

            if (LittleFS.exists("/config.html")) { 

                File file = LittleFS.open("/config.html", "r"); 

                server.streamFile(file, "text/html"); 

                file.close(); 

            } else { 

                server.send(404, "text/plain", "Error: config.html not found in LittleFS!"); 

            } 

        }); 

 

        server.on("/save", HTTP_POST, handleSave); 

         

        // Редірект для Captive Portal (Android/iOS) 

        server.onNotFound([]() { 

            server.sendHeader("Location", String("http://") + apIP.toString(), true); 

            server.send(302, "text/plain", ""); 

        }); 

 

        server.begin(); 

        Serial.println("\nCaptive Portal Active: 192.168.1.1"); 

    } else { 

        Serial.println("\nWiFi Connected! IP: " + WiFi.localIP().toString()); 

    } 

} 

 

void loop() { 

    if (WiFi.getMode() == WIFI_AP) { 

        dnsServer.processNextRequest(); 

    } 

    server.handleClient(); 

} 
