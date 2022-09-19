#ifdef ESP32

#pragma once
#include <Arduino.h>

#define WIFI_NAME "Upgrade Energy Tech"
#define WIFI_PASSWORD "1234"

class Firmware_updater {
public:
    void init_firmware_update_system(void);

    void set_seconds_to_be_active(int seconds);

    void run_pending_tasks();

private:
    bool should_run_forever = true;
    
    uint32_t ms_since_last_check = millis();
    int seconds_until_deactivation = 0;
};

#endif // ESP32