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

#include "interface.h"

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
}

void interface_info_dwn(void)
{
}

void interface_info_display(void)
{
    char data_chars[10];
    display_menu_headline(interface_get_label_text(&interface_text_headline_info), true, 0);

    display_text_line_column(interface_get_label_text(&interface_text_info_num_keys), 2, 1, false);

    sprintf(data_chars, "%u", ena_storage_beacons_count());
    display_text_line_column(data_chars, 2, 9, false);

    ena_exposure_summary_t *current_exposure_summary = ena_exposure_current_summary();

    display_text_line_column(interface_get_label_text(&interface_text_info_exp_days), 4, 1, false);
    sprintf(data_chars, "%d", current_exposure_summary->days_since_last_exposure);
    if (current_exposure_summary->days_since_last_exposure < 0)
    {
        display_text_line_column("/", 4, 12, false);
    }
    else
    {
        display_text_line_column(data_chars, 4, 12, false);
    }

    display_text_line_column(interface_get_label_text(&interface_text_info_exp_num), 5, 1, false);
    sprintf(data_chars, "%d", current_exposure_summary->num_exposures);
    display_text_line_column(data_chars, 5, 12, false);

    display_text_line_column(interface_get_label_text(&interface_text_info_exp_max), 6, 1, false);
    sprintf(data_chars, "%d", current_exposure_summary->max_risk_score);
    display_text_line_column(data_chars, 6, 12, false);

    display_text_line_column(interface_get_label_text(&interface_text_info_exp_sum), 7, 1, false);
    sprintf(data_chars, "%d", current_exposure_summary->risk_score_sum);
    display_text_line_column(data_chars, 7, 12, false);
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
    interface_set_display_function(&interface_info_display);
    interface_set_display_refresh_function(NULL);

    ESP_LOGD(INTERFACE_LOG, "start info interface");
}
