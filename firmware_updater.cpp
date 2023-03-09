// #ifdef ESP32

#include "firmware_updater.h"
#include <esp_wifi.h>

#include <WiFi.h>
#include <WiFiAP.h>
#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include "AsyncElegantOTA.h"

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

static inline bool is_NOT_time_to_execute(uint32_t& ms_since_last_call, uint32_t time_between_calls) {
    if (millis() - ms_since_last_call >= time_between_calls ) {
      ms_since_last_call = millis();
      return false;
    }
    return true;
}

void Firmware_updater::init_firmware_update_system(void) {
    Serial.begin(115200);

    WiFi.setTxPower(WIFI_POWER_19_5dBm);
    WiFi.mode(WIFI_AP);
       // You can remove the password parameter if you want the AP to be open.
    if (!WiFi.softAPConfig(local_IP, gateway, subnet)) {
      Serial.println("STA Failed to configure");
    }
    
    WiFi.softAP(ssid, password);
    
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->redirect("/update");
    });
    
    AsyncElegantOTA.begin(&server);    // Start AsyncElegantOTA
    
    server.begin();
    Serial.println("HTTP server started");
    is_active = true;
}

void Firmware_updater::set_seconds_to_be_active_without_client(int seconds) {
    should_run_forever = false;
    seconds_until_deactivation = seconds;
}

void Firmware_updater::set_stop_only_after_client_disconnection() {
    should_stop_only_after_client_disconnection = true;
}

bool is_there_any_client_connected() {
    return WiFi.softAPgetStationNum() > 0;
}

void Firmware_updater::run_pending_tasks() {
    if (is_NOT_time_to_execute(ms_since_last_check, 1000))
        return;

    if (should_stop_firmware_updater_by_client_disconnection()) {
        stop_firmware_updater();
        return;
    }

    if (should_run_forever)
        return;

    if (should_stop_firmware_updater_by_time()) {
        stop_firmware_updater();
        return;
    }
}

bool Firmware_updater::should_stop_firmware_updater_by_client_disconnection() {
    if (not should_stop_only_after_client_disconnection)
        return false;

    if (is_there_any_client_connected())
        was_there_any_client = true;
    else if (was_there_any_client)
        return true;
    
    return false;
}

bool Firmware_updater::should_stop_firmware_updater_by_time() {
    if (was_there_any_client)
        return false;
    
    if (seconds_until_deactivation <= 0) {
        seconds_until_deactivation = 0;
        stop_firmware_updater();
        return true;
    }

    --seconds_until_deactivation;
    return false;
}

void Firmware_updater::stop_firmware_updater() {
    server.end();
    esp_wifi_stop();
    esp_wifi_deinit();
    is_active = false;

    static bool running = true;
    if (running) {
        running = false;
        Serial.println("Server deactivated");
    }
}

#endif // ESP32