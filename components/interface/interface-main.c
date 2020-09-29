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

#include "display.h"
#include "display-gfx.h"

#include "wifi-controller.h"
#include "ena-storage.h"
#include "ena-exposure.h"

#include "interface.h"

static time_t current_timstamp;
static struct tm *current_tm;
static char time_buffer[32];
static char text_buffer[32];
static bool wifi_connected;

void interface_main_set(void)
{
    interface_datetime_start();
}

void interface_main_rst(void)
{
    interface_report_start();
}

void interface_main_display(void)
{
    if (wifi_connected)
    {
        display_data( display_gfx_wifi, 8, 0, 0, false);
    }
    else
    {
        display_data( display_gfx_cross, 8, 0, 0, false);
    }

    time(&current_timstamp);
    current_tm = localtime(&current_timstamp);

    // curent date
    sprintf(text_buffer, "%s %s %d",
            interface_get_label_text(&interface_texts_weekday[current_tm->tm_wday]),
            interface_get_label_text(&interface_texts_month[current_tm->tm_mon]),
            current_tm->tm_mday);
    display_text_line_column( text_buffer, 0, 16 - strlen(text_buffer), false);

    // current time
    strftime(time_buffer, 16, INTERFACE_FORMAT_TIME, current_tm);
    display_text_line_column( time_buffer, 1, 16 - strlen(time_buffer), false);
}

void interface_main_start(void)
{

    interface_register_command_callback(INTERFACE_COMMAND_RST, &interface_main_rst);
    interface_register_command_callback(INTERFACE_COMMAND_SET, &interface_main_set);
    interface_register_command_callback(INTERFACE_COMMAND_LFT, NULL);
    interface_register_command_callback(INTERFACE_COMMAND_RHT, NULL);
    interface_register_command_callback(INTERFACE_COMMAND_MID, NULL);
    interface_register_command_callback(INTERFACE_COMMAND_UP, NULL);
    interface_register_command_callback(INTERFACE_COMMAND_DWN, NULL);

    if (wifi_controller_connection() != NULL)
    {
        wifi_connected = true;
    }
    else
    {
        wifi_connected = false;
    }

    interface_set_display_function(&interface_main_display);

    ena_exposure_summary_t *current_exposure_summary = ena_exposure_current_summary();

    time(&current_timstamp);
    uint32_t last_update = ena_storage_read_last_exposure_date();

    // status unknown if no update or last update older than two days
    if (last_update == 0 || ((current_timstamp - last_update) / (60 * 60 * 24)) > 2)
    {
        display_data( display_gfx_question[0], 24, 0, 12, false);
        display_data( display_gfx_question[1], 24, 1, 12, false);
        display_data( display_gfx_question[2], 24, 2, 12, false);
        display_data( display_gfx_question[3], 24, 3, 12, false);
    }
    else if (current_exposure_summary->max_risk_score < 100)
    {
        display_data( display_gfx_smile[0], 24, 0, 12, false);
        display_data( display_gfx_smile[1], 24, 1, 12, false);
        display_data( display_gfx_smile[2], 24, 2, 12, false);
        display_data( display_gfx_smile[3], 24, 3, 12, false);
    }
    else
    {
        display_data( display_gfx_sad[0], 24, 0, 12, false);
        display_data( display_gfx_sad[1], 24, 1, 12, false);
        display_data( display_gfx_sad[2], 24, 2, 12, false);
        display_data( display_gfx_sad[3], 24, 3, 12, false);
    }
    // clock icon
    display_data( display_gfx_clock, 8, 4, 8, false);

    // last update
    struct tm *last_update_tm = localtime((time_t*) &last_update);

    sprintf(time_buffer, "%02d %s %02d:%02d",
            last_update_tm->tm_mday,
            interface_get_label_text(&interface_texts_month[last_update_tm->tm_mon]),
            last_update_tm->tm_hour,
            last_update_tm->tm_min);

    if (last_update != 0)
    {
        display_text_line_column( time_buffer, 4, 3, false);
    }

    // buttons
    display_set_button( interface_get_label_text(&interface_text_button_menu), true, false);
    display_set_button( interface_get_label_text(&interface_text_button_report), false, true);

    ESP_LOGD(INTERFACE_LOG, "start main interface");
}
