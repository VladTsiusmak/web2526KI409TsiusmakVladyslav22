#include <Arduino.h> 

#include <WiFi.h> 

#include <LittleFS.h> 

#include <HTTPSServer.hpp> 

#include <SSLCert.hpp> 

#include <HTTPRequest.hpp> 

#include <HTTPResponse.hpp> 

#include <ArduinoJson.h> 

 

using namespace httpsserver; 

 

// --- СЕРТИФІКАТИ (ВШИТІ ДЛЯ НАДІЙНОСТІ) --- 

const char* cert_pem =  

"-----BEGIN CERTIFICATE-----\n" 

"MIIDDTCCAfWgAwIBAgIUC/C+9iPOSQx1lU4G4z5cS18RMKUwDQYJKoZIhvcNAQEL\n" 

"BQAwFjEUMBIGA1UEAwwLMTkyLjE2OC4xLjEwHhcNMjYwMjI1MTM0MjU3WhcNMjcw\n" 

"MjI1MTM0MjU3WjAWMRQwEgYDVQQDDAsxOTIuMTY4LjEuMTCCASIwDQYJKoZIhvcN\n" 

"AQEBBQADggEPADCCAQoCggEBAKrB1lRUbfFmEON1HzS0UIta3zsn3UfqfyLgded9\n" 

"OwI2dVFp3paVIQ+UKulLwMpoVNJmqOJoGKpsuTKPWw6UyZjft5Njv4YuWvxA4rcY\n" 

"a+AgbOjTV96XJMYGYv1L5GVEljEfd9VAzo0AoAjBMcjYJYEKDzshyJemMGI0smis\n" 

"sUae88fPYnyOns3w7AL0q2Q9HDO4le4xA6nTzCubyQ4OGQElH4l8h9mrOsqTM4Dv\n" 

"FluzdshevvdXzvbWUq2AEnJZsJx6IL+O7g8bl1/DMZvIU2nNsTjZU89F+/9O45qi\n" 

"u393pA9uoL36Jrh+Wd0atGrDsT1KJOQkh2DMwW3fMY+HMQ0CAwEAAaNTMFEwHQYD\n" 

"VR0OBBYEFEbWmVSzcRCmCLlgm22WAvGo/X/eMB8GA1UdIwQYMBaAFEbWmVSzcRCm\n" 

"CLlgm22WAvGo/X/eMA8GA1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQELBQADggEB\n" 

"AIbXkC9+i34V0i1pgjZn85t1iQ/9t8YPPzkxECkHtkLONQ6fXfHFA5AM/gxeCaSO\n" 

"XFW/+y6VuQ8sFZqq+dObRQWzhxNWm7CaSCKdFKajcbq7fnLnwb1B5yE0XagyIlCg\n" 

"lwly56CkLgaWX8gzfHWd2K2x6EaER8HDxYIA94l+FXg/a+ZXVjVNG86U+Y8YVbLL\n" 

"yhNsfc8TAdiJMtJBxk52JUB0JDk6mKJ/6OS7zAKwEBc1bEMmX4VBvnEIc8P0jzkL\n" 

"l9Iv/gBFbeJQE8ZKvCtlJHxFllFywEFtlVbI2vneV/LLKzNG+lNAlrhtzBycIguO\n" 

"Kwrki4yJCxRI/DG1Cq3i/gk=\n" 

"-----END CERTIFICATE-----"; 

 

const char* key_pem =  

"-----BEGIN PRIVATE KEY-----\n" 

"MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQCqwdZUVG3xZhDj\n" 

"dR80tFCLWt87J91H6n8i4HXnfTsCNnVRad6WlSEPlCrpS8DKaFTSZqjiaBiqbLky\n" 

"j1sOlMmY37eTY7+GLlr8QOK3GGvgIGzo01felyTGBmL9S+RlRJYxH3fVQM6NAKAI\n" 

"wTHI2CWBCg87IciXpjBiNLJorLFGnvPHz2J8jp7N8OwC9KtkPRwzuJXuMQOp08wr\n" 

"m8kODhkBJR+JfIfZqzrKkzOA7xZbs3bIXr73V8721lKtgBJyWbCceiC/ju4PG5df\n" 

"wzGbyFNpzbE42VPPRfv/TuOaort/d6QPbqC9+ia4flndGrRqw7E9SiTkJIdgzMFt\n" 

"3zGPhzENAgMBAAECggEACmYJflMx6sxzZNaiEtyUNxhOldSkpheCrIYMyA8ZQ9Y7\n" 

"KL0QwAzO2ZsFTfW+BNV7rOBxknASaHZ8hLa0LdwjtrqQ/FnLtQkno0r0Q4AqOZjf\n" 

"6QHNzRFZPPHwn0o622vk1ZL8AjGTIoUSOdZCCOAY0JXjPY+eHpYRRjm9T6CMnyc/\n" 

"WY4LTTFw5fbRKLEHw8sXssRtgAY94ZJOOkv00Ec/I4jSJsb4wq/xDhZ4tfKcOWrK\n" 

"aC/kCnlgSUTvxXJiKZ27vpDaocONdZjfl6jlWa/ShIFJm+quDZfkyY7LCgT+WXU4\n" 

"jyvTE/IdRGf+kNDxJ6eAFBiPftnxXeBlt41ITUblJQKBgQDhhfVCKMqmZnpYklT9\n" 

"W4Xr8QAfdH+88ldtEKUBbaIKeoXwsjsQ/whQXlkyygpYbiAaVrwZL5jKy4CCDzR9\n" 

"2anRKUJ2n0IMDZXFj9gKmZDKvpAXX7z5swgo2InxENcahUwEp0JFbw2brOw1b/au\n" 

"Tvok2dWdDseBlIbuk2zR67/nlwKBgQDB1Tr+UMRp2XdGDJxQprZStxH9ugu5zbtf\n" 

"xbRRtt5mOaa1P70qB/D+8nJmCRsgESy90P+01K0zFN7TGGqF6Ev6HWsIDtdbKjuo\n" 

"FYxIQx6yZ+bObu7wk0v57gdMap6C/izGs8ZZ6W9ys3vi0Z9PpNJacira37oElz2T\n" 

"ObN9TX3g+wKBgQDdBM/MOvYABka5LxAkDOI+zT31ldLqdgbAWKOdtrgr1NAXiFTi\n" 

"305ilVSNtfxVlffYWFCXzlHLzYzv6b7j58HJvsnoP0QXU+kIaWStXrrMN943ShiG\n" 

"DrNcdHsbyglFcIWDuff03FVV0C7eYBA93CDCpMn41lcn40wp3Em1oXm9mQKBgCOM\n" 

"KnSHdW4AHtqu51/3IwrKXjdNDmd17n4i8O3nfqP6wLMOzG9P44euCQLjeqEH5U+u\n" 

"a6bLNjRfCHpoSOjj8EMfcAibSC2E8kniIccC4DdRzMdhGtrt+IoOeDIwf3+Ag8+x\n" 

"GphUskaz2e2AChp+HMHPnEJls2SyI+3DMEKzmnlPAoGAb7IJ27v4J2jY3z1XpKqB\n" 

"U5qIg+yfXE9ioKU6YD940nMSgwFDaKBbcgxBmrVT7QUV4DNn3jXE/dxahALNkSUc\n" 

"R75yPaf8YOmuEZG/MhXMiwRmsNTUMM1IXBn9nx+IFcb2HqTiJS1XEOi9yLBXe1TF\n" 

"m9UVdOKMhlPJZUxBhfp9Lkg=\n" 

"-----END PRIVATE KEY-----"; 

 

// --- НАЛАШТУВАННЯ --- 

const char* ssid = "miron_shtyrm"; 

const char* password = "miron2019"; 

const char* auth_hash = "Basic YWRtaW46cGFzc3dvcmQ="; // admin:password 

const int LED_PIN = 2; 

bool ledState = false; 

 

HTTPSServer * secureServer; 

SSLCert * cert; 

 

// --- ОБРОБНИКИ --- 

 

void handleRoot(HTTPRequest * req, HTTPResponse * res) { 

    if (req->getHeader("Authorization") != auth_hash) { 

        res->setStatusCode(401); 

        res->setHeader("WWW-Authenticate", "Basic realm=\"Secure Miron Core\""); 

        res->print("401 Unauthorized"); 

        return; 

    } 

 

    File file = LittleFS.open("/index.html", "r"); 

    if (file) { 

        res->setHeader("Content-Type", "text/html"); 

        size_t fileSize = file.size(); 

        uint8_t * buffer = (uint8_t *)malloc(fileSize); 

        if (buffer) { 

            file.read(buffer, fileSize); 

            res->write(buffer, fileSize);  

            free(buffer); 

        } 

        file.close(); 

    } else { 

        res->setStatusCode(404); 

        res->print("HTML File not found in LittleFS"); 

    } 

} 

 

void handleStatus(HTTPRequest * req, HTTPResponse * res) { 

    JsonDocument doc; 

    doc["led_on"] = ledState; 

    String responseText; 

    serializeJson(doc, responseText); 

    res->setHeader("Content-Type", "application/json"); 

    res->print(responseText); 

} 

 

void handleControl(HTTPRequest * req, HTTPResponse * res) { 

    char buffer[128]; 

    size_t bytesRead = req->readChars(buffer, 128); 

    buffer[bytesRead] = '\0'; 

 

    JsonDocument doc; 

    deserializeJson(doc, buffer); 

     

    if (doc["command"] == "on") { 

        ledState = true; 

        digitalWrite(LED_PIN, HIGH); 

    } else if (doc["command"] == "off") { 

        ledState = false; 

        digitalWrite(LED_PIN, LOW); 

    } 

    res->print("OK"); 

} 

 

void setup() { 

    Serial.begin(115200); 

    pinMode(LED_PIN, OUTPUT); 

 

    if (!LittleFS.begin(true)) { 

        Serial.println("LittleFS Mount Failed"); 

    } 

 

    WiFi.begin(ssid, password); 

    while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); } 

    Serial.println("\nConnected! IP: " + WiFi.localIP().toString()); 

 

    // Використання вшитих сертифікатів з коректним завершенням рядка 

    cert = new SSLCert((unsigned char*)cert_pem, strlen(cert_pem) + 1,  

                       (unsigned char*)key_pem, strlen(key_pem) + 1); 

     

    secureServer = new HTTPSServer(cert); 

 

    ResourceNode * nodeRoot    = new ResourceNode("/", "GET", &handleRoot); 

    ResourceNode * nodeStatus  = new ResourceNode("/api/status", "GET", &handleStatus); 

    ResourceNode * nodeControl = new ResourceNode("/api/control", "POST", &handleControl); 

     

    secureServer->registerNode(nodeRoot); 

    secureServer->registerNode(nodeStatus); 

    secureServer->registerNode(nodeControl); 

 

    secureServer->start(); 

    if (secureServer->isRunning()) { 

        Serial.println("HTTPS Server ACTIVE (Certificates Validated)"); 

    } 

} 

 

void loop() { 

    if (secureServer != nullptr && secureServer->isRunning()) { 

        secureServer->loop(); 

    } 

} 
