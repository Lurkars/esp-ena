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
#include "esp_log.h"

#include "ena.h"
#include "ena-storage.h"
#include "ena-beacons.h"
#include "ena-exposure.h"
#include "ena-bluetooth-advertise.h"
#include "ena-bluetooth-scan.h"
#include "ena-cwa.h"
#include "interface.h"
#include "ds3231.h"
#include "ssd1306.h"
#include "wifi-controller.h"

#include "sdkconfig.h"

void app_main(void)
{

    // debug only own LOG TAGs
    esp_log_level_set("*", ESP_LOG_WARN);
    esp_log_level_set(ENA_LOG, ESP_LOG_DEBUG);
    esp_log_level_set(ENA_BEACON_LOG, ESP_LOG_DEBUG);
    esp_log_level_set(ENA_ADVERTISE_LOG, ESP_LOG_DEBUG);
    esp_log_level_set(ENA_SCAN_LOG, ESP_LOG_DEBUG);
    esp_log_level_set(ENA_EXPOSURE_LOG, ESP_LOG_DEBUG);
    esp_log_level_set(ENA_STORAGE_LOG, ESP_LOG_INFO);
    esp_log_level_set(ENA_CWA_LOG, ESP_LOG_DEBUG);
    esp_log_level_set(INTERFACE_LOG, ESP_LOG_DEBUG);
    esp_log_level_set(WIFI_LOG, ESP_LOG_DEBUG);

    // start interface
    interface_start();

    // set system time from DS3231
    struct tm rtc_time;
    ds3231_get_time(&rtc_time);

    time_t curtime = mktime(&rtc_time);
    struct timeval tv = {0};
    tv.tv_sec = curtime;
    settimeofday(&tv, NULL);

    // Hardcoded timezone of UTC+2 for now (consider POSIX notation!)
    setenv("TZ", "UTC-2", 1);
    tzset();

    ena_start();

    // start with main interface
    interface_main_start();

    while (1)
    {
        ena_run();
        ena_cwa_run();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
