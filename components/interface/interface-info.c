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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"

#include "display.h"
#include "display-gfx.h"

#include "ena-storage.h"
#include "ena-exposure.h"
#include "ena-bluetooth-scan.h"

#include "interface.h"

typedef enum
{
    INTERFACE_INFO_STATUS_EXPOSURE = 0,
    INTERFACE_INFO_STATUS_BEACONS,
    INTERFACE_INFO_STATUS_SCAN,
    INTERFACE_INFO_STATUS_SYSTEM
} info_status_e;

static int current_info_status = INTERFACE_INFO_STATUS_EXPOSURE;

void interface_info_set(void)
{
    interface_main_start();
}

void interface_info_rst(void)
{
}

void interface_info_lft(void)
{
    interface_data_start();
}

void interface_info_rht(void)
{
    interface_settings_start();
}

void interface_info_mid(void)
{
}

void interface_info_up(void)
{
    current_info_status--;
    if (current_info_status < INTERFACE_INFO_STATUS_EXPOSURE)
    {
        current_info_status = INTERFACE_INFO_STATUS_SYSTEM;
    }
}

void interface_info_dwn(void)
{
    current_info_status++;
    if (current_info_status > INTERFACE_INFO_STATUS_SYSTEM)
    {
        current_info_status = INTERFACE_INFO_STATUS_EXPOSURE;
    }
}

int interface_info_num_offset(int num)
{
    if (num < 10)
    {
        return 13;
    }
    else if (num < 100)
    {
        return 12;
    }
    else
    {
        return 11;
    }
}

void interface_info_display(void)
{
    display_clear();
    char char_buffer[64];
    display_menu_headline(interface_get_label_text(&interface_text_headline_info), true, 0);

    display_data(display_gfx_arrow_up, 8, 1, 60, false);

    if (current_info_status == INTERFACE_INFO_STATUS_EXPOSURE)
    {
        ena_exposure_summary_t *current_exposure_summary = ena_exposure_current_summary();

        display_text_line_column(interface_get_label_text(&interface_text_info_exp_days), 3, 1, false);
        int last = current_exposure_summary->days_since_last_exposure;
        if (last >= 0)
        {
            sprintf(char_buffer, "%d", last);
            display_text_line_column(char_buffer, 3, interface_info_num_offset(last), false);
        }

        display_text_line_column(interface_get_label_text(&interface_text_info_exp_num), 4, 1, false);
        sprintf(char_buffer, "%d", current_exposure_summary->num_exposures);
        display_text_line_column(char_buffer, 4, interface_info_num_offset(current_exposure_summary->num_exposures), false);

        display_text_line_column(interface_get_label_text(&interface_text_info_exp_max), 5, 1, false);
        sprintf(char_buffer, "%d", current_exposure_summary->max_risk_score);
        display_text_line_column(char_buffer, 5, interface_info_num_offset(current_exposure_summary->max_risk_score), false);

        display_text_line_column(interface_get_label_text(&interface_text_info_exp_sum), 6, 1, false);
        sprintf(char_buffer, "%d", current_exposure_summary->risk_score_sum);
        display_text_line_column(char_buffer, 6, interface_info_num_offset(current_exposure_summary->risk_score_sum), false);
    }
    else if (current_info_status == INTERFACE_INFO_STATUS_BEACONS)
    {
        display_text_line_column(interface_get_label_text(&interface_text_info_num_keys), 3, 1, false);

        int num = ena_storage_beacons_count();
        sprintf(char_buffer, "%u", num);
        display_text_line_column(char_buffer, 3, interface_info_num_offset(num), false);

        display_text_line_column(interface_get_label_text(&interface_text_info_last_keys), 4, 1, false);

        time_t current_timstamp;
        time(&current_timstamp);

        int min = ena_expore_check_find_min((uint32_t)current_timstamp - 60 * 30);
        int last30 = num - min;

        if (last30 >= 0)
        {
            sprintf(char_buffer, "%d", last30);
            display_text_line_column(char_buffer, 4, interface_info_num_offset(last30), false);
        }
    }
    else if (current_info_status == INTERFACE_INFO_STATUS_SCAN)
    {
        int current_scan = ena_bluetooth_scan_get_last_num();

        switch (ena_bluetooth_scan_get_status())
        {
        case ENA_SCAN_STATUS_SCANNING:
            display_text_line_column(interface_get_label_text(&interface_text_info_scan_status_scanning), 3, 1, false);
            break;
        case ENA_SCAN_STATUS_NOT_SCANNING:
            display_text_line_column(interface_get_label_text(&interface_text_info_scan_status_notscanning), 3, 1, false);
            break;
        case ENA_SCAN_STATUS_WAITING:
            display_text_line_column(interface_get_label_text(&interface_text_info_scan_status_waiting), 3, 1, false);
            break;
        }

        display_text_line_column(interface_get_label_text(&interface_text_info_scan_last), 5, 1, false);
        sprintf(char_buffer, "%d", current_scan);
        display_text_line_column(char_buffer, 5, interface_info_num_offset(current_scan), false);
    }
    else if (current_info_status == INTERFACE_INFO_STATUS_SYSTEM)
    {
    }

    display_data(display_gfx_arrow_down, 8, 7, 60, false);
}

void interface_info_start(void)
{

    interface_register_command_callback(INTERFACE_COMMAND_RST, &interface_info_rst);
    interface_register_command_callback(INTERFACE_COMMAND_SET, &interface_info_set);
    interface_register_command_callback(INTERFACE_COMMAND_LFT, &interface_info_lft);
    interface_register_command_callback(INTERFACE_COMMAND_RHT, &interface_info_rht);
    interface_register_command_callback(INTERFACE_COMMAND_MID, &interface_info_mid);
    interface_register_command_callback(INTERFACE_COMMAND_UP, &interface_info_up);
    interface_register_command_callback(INTERFACE_COMMAND_DWN, &interface_info_dwn);
    interface_register_command_callback(INTERFACE_COMMAND_RST_LONG, NULL);
    interface_register_command_callback(INTERFACE_COMMAND_SET_LONG, NULL);

    interface_set_display_function(&interface_info_display);
}
