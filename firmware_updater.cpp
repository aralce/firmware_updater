#include "firmware_updater.h"
#include <esp_wifi.h>

#include <Arduino.h>

#include <WiFi.h>
#include <WiFiAP.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>

const char* ssid = WIFI_NAME;
const char* password = WIFI_PASSWORD;

AsyncWebServer server(80);

// Set your Static IP address
IPAddress local_IP(192, 168, 1, 184);
// Set your Gateway IP address
IPAddress gateway(192, 168, 1, 1);

IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);   //optional
IPAddress secondaryDNS(8, 8, 4, 4); //optional

static inline bool is_time_to_execute(uint32_t& ms_since_last_call, uint32_t time_between_calls) {
    if (millis() - ms_since_last_call >= time_between_calls) {
      ms_since_last_call = millis();
      return true;
    }
    return false;
}

void Firmware_updater::init_firmware_update_system(void) {
    Serial.begin(115200);
    
    WiFi.mode(WIFI_AP);
       // You can remove the password parameter if you want the AP to be open.
    if (!WiFi.softAPConfig(local_IP, gateway, subnet)) {
      Serial.println("STA Failed to configure");
    }
    
    WiFi.softAP(ssid, password);
    
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    server.begin();
       server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->redirect("/update");
    });
    
    AsyncElegantOTA.begin(&server);    // Start AsyncElegantOTA
    
    server.begin();
    Serial.println("HTTP server started");
}

void Firmware_updater::set_seconds_to_be_active(int seconds) {
    should_run_forever = false;
    seconds_until_deactivation = seconds;
}

void Firmware_updater::run_pending_tasks() {
    if (!is_time_to_execute(ms_since_last_check, 1000))
        return;
    if (should_run_forever)
        return;
    
    if (seconds_until_deactivation <= 0) {
        seconds_until_deactivation = 0;
        server.end();
        esp_wifi_stop();
        Serial.println("Server deactivated");
        return;
    }
    --seconds_until_deactivation;
}
