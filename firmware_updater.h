#ifdef ESP32

#pragma once
#include <stdint.h>
#include <HAL_system/HAL_system_singleton.h>

#define WIFI_NAME "Upgrade Energy Tech"
#define WIFI_PASSWORD "123456789"

class Firmware_updater {
public:
    void init_firmware_update_system(void);

    void set_seconds_to_be_active_without_client(int seconds);

    void set_stop_only_after_client_disconnection();

    bool is_server_ACTIVE() { return is_active;}

    bool is_server_INACTIVE() { return not is_active;}

    void run_pending_tasks();

private:
    bool should_run_forever = true;
    bool should_stop_only_after_client_disconnection = false;
    bool was_there_any_client = false;
    
    HAL_system_api* device = HAL_system_singleton::get_HAL_system_instance();
    uint32_t ms_since_last_check = device->millisecs_since_init();
    int seconds_until_deactivation = 0;

    bool is_active = false;

    bool should_stop_firmware_updater_by_client_disconnection();
    bool should_stop_firmware_updater_by_time();
    void stop_firmware_updater();
};

#endif // ESP32