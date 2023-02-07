#ifdef ESP32

#pragma once
#include <Arduino.h>

#define WIFI_NAME "Upgrade Energy Tech"
#define WIFI_PASSWORD "123456789"

class Firmware_updater {
public:
    void init_firmware_update_system(void);

    void set_seconds_to_be_active_without_client(int seconds);

    void set_stop_only_after_client_disconnection();

    void run_pending_tasks();

private:
    bool should_run_forever = true;
    bool should_stop_only_after_client_disconnection = false;
    bool was_there_any_client = false;
    
    uint32_t ms_since_last_check = millis();
    int seconds_until_deactivation = 0;

    bool should_stop_firmware_updater_by_client_disconnection();
};

#endif // ESP32