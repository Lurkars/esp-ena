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
#include <time.h>
#include <sys/time.h>
#include "esp_log.h"
#include "driver/gpio.h"

#include "ds3231.h"
#include "ssd1306.h"
#include "ssd1306-gfx.h"

#include "interface.h"

typedef enum
{
    INTERFACE_DATETIME_STATE_HOUR = 0,
    INTERFACE_DATETIME_STATE_MINUTE,
    INTERFACE_DATETIME_STATE_SECONDS,
    INTERFACE_DATETIME_STATE_DAY,
    INTERFACE_DATETIME_STATE_MONTH,
    INTERFACE_DATETIME_STATE_YEAR,
} interface_datetime_state_t;

static int current_interface_datetime_state;

const uint32_t interface_datetime_steps[6] = {3600, 60, 1, 86400, 2629800, 31557600};

int interface_datetime_state(void)
{
    return current_interface_datetime_state;
}

void interface_datetime_set(void)
{
    interface_main_start();
}

void interface_datetime_lft(void)
{
    interface_settings_start();
}

void interface_datetime_rht(void)
{
    interface_wifi_start();
}

void interface_datetime_mid(void)
{
    current_interface_datetime_state++;
    if (current_interface_datetime_state > INTERFACE_DATETIME_STATE_YEAR)
    {
        current_interface_datetime_state = INTERFACE_DATETIME_STATE_HOUR;
    }
    ESP_LOGD(INTERFACE_LOG, "datetime to %d", current_interface_datetime_state);
}

void interface_datetime_up(void)
{
    time_t curtime = time(NULL);
    curtime += interface_datetime_steps[current_interface_datetime_state];
    struct timeval tv = {0};
    tv.tv_sec = curtime;
    settimeofday(&tv, NULL);
    ds3231_set_time(gmtime(&curtime));
    ESP_LOGD(INTERFACE_LOG, "increment %d about %u %s", current_interface_datetime_state, interface_datetime_steps[current_interface_datetime_state], ctime(&curtime));
}

void interface_datetime_dwn(void)
{
    time_t curtime = time(NULL);
    curtime -= interface_datetime_steps[current_interface_datetime_state];
    struct timeval tv = {0};
    tv.tv_sec = curtime;
    settimeofday(&tv, NULL);
    ds3231_set_time(gmtime(&curtime));
    ESP_LOGD(INTERFACE_LOG, "decrement %d about %u %s", current_interface_datetime_state, interface_datetime_steps[current_interface_datetime_state], ctime(&curtime));
}

void interface_datetime_display(void)
{
    static time_t current_timstamp;
    static struct tm *current_tm;
    static char time_buffer[9];
    static char date_buffer[32];

    ssd1306_menu_headline(SSD1306_ADDRESS, interface_get_label_text(&interface_text_headline_time), true, 0);
    static char edit_char[3];
    static int edit_line = 3;
    int edit_length = 2;
    int edit_offset = 0;

    ssd1306_clear_line(SSD1306_ADDRESS, edit_line - 1, false);
    ssd1306_clear_line(SSD1306_ADDRESS, edit_line + 1, false);

    time(&current_timstamp);
    current_tm = localtime(&current_timstamp);

    strftime(time_buffer, 16, INTERFACE_FORMAT_TIME, current_tm);
    ssd1306_text_line_column(SSD1306_ADDRESS, time_buffer, 3, 4, false);

    sprintf(date_buffer, "%02d %s %02d",
            current_tm->tm_mday,
            interface_get_label_text(&interface_texts_month[current_tm->tm_mon]),
            current_tm->tm_year - 100);
    ssd1306_text_line_column(SSD1306_ADDRESS, date_buffer, 6, 4, false);

    switch (interface_datetime_state())
    {

    case INTERFACE_DATETIME_STATE_YEAR:
        memcpy(&edit_char, &date_buffer[7], edit_length);
        edit_line = 6;
        edit_offset = 11;
        break;
    case INTERFACE_DATETIME_STATE_DAY:
        memcpy(&edit_char, &date_buffer[0], edit_length);
        edit_line = 6;
        edit_offset = 4;
        break;
    case INTERFACE_DATETIME_STATE_MONTH:
        edit_length = 3;
        memcpy(&edit_char, &date_buffer[3], edit_length);
        edit_line = 6;
        edit_offset = 7;
        break;
    case INTERFACE_DATETIME_STATE_HOUR:
        memcpy(&edit_char, &time_buffer[0], edit_length);
        edit_line = 3;
        edit_offset = 4;
        break;
    case INTERFACE_DATETIME_STATE_MINUTE:
        memcpy(&edit_char, &time_buffer[3], edit_length);
        edit_line = 3;
        edit_offset = 7;
        break;
    case INTERFACE_DATETIME_STATE_SECONDS:
        memcpy(&edit_char, &time_buffer[6], edit_length);
        edit_line = 3;
        edit_offset = 10;
        break;
    }

    ssd1306_data(SSD1306_ADDRESS, ssd1306_gfx_arrow_up, 8, edit_line - 1, edit_offset * 8 + 4, false);
    ssd1306_chars(SSD1306_ADDRESS, edit_char, edit_length, edit_line, edit_offset, true);
    ssd1306_data(SSD1306_ADDRESS, ssd1306_gfx_arrow_down, 8, edit_line + 1, edit_offset * 8 + 4, false);
}

void interface_datetime_start(void)
{
    current_interface_datetime_state = INTERFACE_DATETIME_STATE_HOUR;
    interface_register_button_callback(INTERFACE_BUTTON_LFT, &interface_datetime_lft);
    interface_register_button_callback(INTERFACE_BUTTON_RHT, &interface_datetime_rht);
    interface_register_button_callback(INTERFACE_BUTTON_MID, &interface_datetime_mid);
    interface_register_button_callback(INTERFACE_BUTTON_UP, &interface_datetime_up);
    interface_register_button_callback(INTERFACE_BUTTON_DWN, &interface_datetime_dwn);
    interface_register_button_callback(INTERFACE_BUTTON_SET, &interface_datetime_set);
    interface_register_button_callback(INTERFACE_BUTTON_RST, NULL);
    interface_set_display_function(&interface_datetime_display);
    ESP_LOGD(INTERFACE_LOG, "start datetime interface");
}