/**
 * provide bluetooth part of Exposure Notification API v1.2 as defined by Apple/Google
 *
 * Source documents:
 *
 * https://blog.google/documents/70/Exposure_Notification_-_Bluetooth_Specification_v1.2.2.pdf
 *
 * https://covid19-static.cdn-apple.com/applications/covid19/current/static/detection-tracing/pdf/ExposureNotification-BluetoothSpecificationv1.2.pdf
 *
 *
 *
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <time.h>
#include <sys/time.h>
#include "esp_system.h"
#include "esp_log.h"

#include "ena.h"
#include "ena-storage.h"
#include "ena-interface.h"
#include "ena-interface-menu.h"

#include "sdkconfig.h"

void app_main(void)
{
    // DEBUG set time
    struct timeval tv = {1594459800, 0}; // current hardcoded timestamp (2020-07-11 09:30:00) ¯\_(ツ)_/¯
    settimeofday(&tv, NULL);

    esp_log_level_set(ENA_STORAGE_LOG, ESP_LOG_INFO);
   // ena_storage_erase(); // only needed on first start! TODO automatically check

    ena_interface_start();
    ena_interface_menu_start();
    ena_start();
}
