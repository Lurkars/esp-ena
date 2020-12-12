// Copyright 2020 Lukas Haubaum
//
// Licensed under the GNU Affero General Public License, Version 3;
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     https://www.gnu.org/licenses/agpl-3.0.html
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <time.h>
#include <sys/time.h>
#include "esp_system.h"
#include "esp_sntp.h"
#include "esp_log.h"

#include "ena.h"
#include "ena-storage.h"
#include "ena-beacons.h"
#include "ena-exposure.h"
#include "ena-bluetooth-advertise.h"
#include "ena-bluetooth-scan.h"
#include "ena-eke-proxy.h"
#include "interface.h"
#include "rtc.h"
#include "wifi-controller.h"

#include "sdkconfig.h"

#ifdef CONFIG_ENA_INTERFACE_CUSTOM
#include "button-input.h"
#endif

#if defined(CONFIG_ENA_INTERFACE_M5STICKC) || defined(CONFIG_ENA_INTERFACE_M5STICKC_PLUS) 
#include "m5-input.h"
#endif

void time_sync_notification_cb(struct timeval *tv)
{
    time_t time = (time_t)tv->tv_sec;
    struct tm *rtc_time = gmtime(&time);
    rtc_set_time(rtc_time);
    settimeofday(tv, NULL);
    ESP_LOGD(ENA_LOG, "NTP time:%lu %s", tv->tv_sec, asctime(rtc_time));
}

void app_main(void)
{
    // debug only own LOG TAGs
    esp_log_level_set("*", ESP_LOG_WARN);
    esp_log_level_set("wifi", ESP_LOG_ERROR);
    esp_log_level_set(ENA_LOG, ESP_LOG_DEBUG);
    esp_log_level_set(ENA_BEACON_LOG, ESP_LOG_INFO);
    esp_log_level_set(ENA_ADVERTISE_LOG, ESP_LOG_INFO);
    esp_log_level_set(ENA_SCAN_LOG, ESP_LOG_INFO);
    esp_log_level_set(ENA_EXPOSURE_LOG, ESP_LOG_DEBUG);
    esp_log_level_set(ENA_STORAGE_LOG, ESP_LOG_INFO);
    esp_log_level_set(ENA_EKE_PROXY_LOG, ESP_LOG_DEBUG);
    esp_log_level_set(INTERFACE_LOG, ESP_LOG_INFO);
    esp_log_level_set(WIFI_LOG, ESP_LOG_INFO);

    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
    sntp_init();

    // set system time from RTC
    struct tm rtc_time;
    rtc_get_time(&rtc_time);

    time_t curtime = mktime(&rtc_time);
    struct timeval tv = {0};
    tv.tv_sec = curtime;
    settimeofday(&tv, NULL);

    ena_start();

    // start interface
    interface_start();

    // start with main interface
    interface_main_start();

    // start button input
#if defined(CONFIG_ENA_INTERFACE_CUSTOM)
    button_input_start();
#endif

#if defined(CONFIG_ENA_INTERFACE_M5STICKC) || defined(CONFIG_ENA_INTERFACE_M5STICKC_PLUS) 
    m5_input_start();
#endif

    wifi_controller_reconnect(NULL);

    while (1)
    {
        ena_run();
        ena_eke_proxy_run();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
