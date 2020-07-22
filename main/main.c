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
#include <stdio.h>
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
#include "ena-interface.h"
#include "ena-interface-menu.h"
#include "ena-interface-datetime.h"
#include "ssd1306.h"
#include "ds3231.h"

#include "sdkconfig.h"

static time_t curtime;

void interface_display_time(void *pvParameter)
{
    static char *curtime_text;
    static struct tm rtc_time;
    static bool edit_invert = false;
    while (1)
    {
        curtime = time(NULL);
        localtime_r(&curtime, &rtc_time);
        curtime_text = asctime(&rtc_time);
        ssd1306_text_line(SSD1306_ADDRESS, curtime_text, 0, false);
        gmtime_r(&curtime, &rtc_time);
        curtime_text = asctime(&rtc_time);
        ssd1306_text_line(SSD1306_ADDRESS, curtime_text, 1, false);
        if (ena_interface_get_state() == ENA_INTERFACE_STATE_SET_DATETIME)
        {
            edit_invert = !edit_invert;
            ds3231_set_time(&rtc_time);
            char edit_year[4] = "";
            char edit_month[3] = "";
            char edit_day[2] = "";
            char edit_hour[2] = "";
            char edit_minute[2] = "";
            char edit_second[2] = "";
            switch (ena_interface_datetime_state())
            {

            case ENA_INTERFACE_DATETIME_STATE_YEAR:
                memcpy(&edit_year, &curtime_text[20], 4);
                ssd1306_text_line_column(SSD1306_ADDRESS, edit_year, 0, 20, edit_invert);
                break;
            case ENA_INTERFACE_DATETIME_STATE_MONTH:
                memcpy(&edit_month, &curtime_text[4], 3);
                ssd1306_text_line_column(SSD1306_ADDRESS, edit_month, 0, 4, edit_invert);
                break;
            case ENA_INTERFACE_DATETIME_STATE_DAY:
                memcpy(&edit_day, &curtime_text[8], 2);
                ssd1306_text_line_column(SSD1306_ADDRESS, edit_day, 0, 8, edit_invert);
                break;
            case ENA_INTERFACE_DATETIME_STATE_HOUR:
                memcpy(&edit_hour, &curtime_text[11], 2);
                ssd1306_text_line_column(SSD1306_ADDRESS, edit_hour, 0, 11, edit_invert);
                break;
            case ENA_INTERFACE_DATETIME_STATE_MINUTE:
                memcpy(&edit_minute, &curtime_text[14], 2);
                ssd1306_text_line_column(SSD1306_ADDRESS, edit_minute, 0, 14, edit_invert);
                break;
            case ENA_INTERFACE_DATETIME_STATE_SECONDS:
                memcpy(&edit_second[0], &curtime_text[17], 2);
                ssd1306_text_line_column(SSD1306_ADDRESS, edit_second, 0, 17, edit_invert);
                break;
            }
        }
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void interface_display_status(void *pvParameter)
{
    static bool get_status = true;
    while (1)
    {
        if (ena_interface_get_state() == ENA_INTERFACE_STATE_STATUS)
        {
            if (get_status)
            {
                ena_exposure_summary_t summary;
                ena_exposure_summary(ena_exposure_default_config(), &summary);
                char buffer[23];
                sprintf(buffer, "Days: %d", summary.days_since_last_exposure);
                ssd1306_text_line(SSD1306_ADDRESS, buffer, 3, false);
                sprintf(buffer, "Exposures: %d", summary.num_exposures);
                ssd1306_text_line(SSD1306_ADDRESS, buffer, 4, false);
                sprintf(buffer, "Score: %d, Max: %d", summary.risk_score_sum, summary.max_risk_score);
                ssd1306_text_line(SSD1306_ADDRESS, buffer, 5, false);
                get_status = false;
            }
        }
        else if (!get_status)
        {
            ssd1306_clear_line(SSD1306_ADDRESS, 2, false);
            ssd1306_clear_line(SSD1306_ADDRESS, 3, false);
            ssd1306_clear_line(SSD1306_ADDRESS, 4, false);
            get_status = true;
        }
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void interface_display_idle(void *pvParameter)
{
    static bool set_status = true;
    while (1)
    {
        if (ena_interface_get_state() == ENA_INTERFACE_STATE_IDLE)
        {
            if (set_status)
            {
                ssd1306_on(SSD1306_ADDRESS, false);
                set_status = false;
            }
        }
        else if (!set_status)
        {
            ssd1306_on(SSD1306_ADDRESS, true);
            set_status = true;
        }
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    struct tm rtc_time;
    ds3231_get_time(&rtc_time);
    curtime = mktime(&rtc_time);
    struct timeval tv = {0};
    tv.tv_sec = curtime;
    settimeofday(&tv, NULL);
    esp_log_level_set(ENA_STORAGE_LOG, ESP_LOG_INFO);

    setenv("TZ", "UTC-2", 1);
    tzset();

    ssd1306_start(SSD1306_ADDRESS);
    ssd1306_clear(SSD1306_ADDRESS);

    ena_interface_start();
    ena_interface_menu_start();

    ena_start();

    xTaskCreate(&interface_display_time, "interface_display_time", 4096, NULL, 5, NULL);
    xTaskCreate(&interface_display_status, "interface_display_status", 4096, NULL, 5, NULL);
    xTaskCreate(&interface_display_idle, "interface_display_idle", 4096, NULL, 5, NULL);
}
