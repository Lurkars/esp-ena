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

#include "interface.h"

typedef enum
{
    INTERFACE_DATA_DEL_TEK = 0,
    INTERFACE_DATA_DEL_EXPOSURE_INFO,
    INTERFACE_DATA_DEL_TEMP_RPI,
    INTERFACE_DATA_DEL_RPI,
    INTERFACE_DATA_DEL_LAST_EXPOSURE,
    INTERFACE_DATA_DEL_ALL,
} interface_data_state_t;

static int current_interface_data_state;
static int current_data_index;
static bool confirm_current;

void interface_data_set(void)
{
    if (!confirm_current)
    {
        interface_main_start();
    }
    else
    {
        confirm_current = false;
        display_clear_line( 2, false);
        display_clear_line( 4, false);
        display_clear_line( 6, false);
    }
}

void interface_data_rst(void)
{

    if (confirm_current)
    {
        switch (current_interface_data_state)
        {
        case INTERFACE_DATA_DEL_TEK:
            ena_storage_erase_tek();
            break;
        case INTERFACE_DATA_DEL_EXPOSURE_INFO:
            ena_storage_erase_exposure_information();
            break;
        case INTERFACE_DATA_DEL_TEMP_RPI:
            ena_storage_erase_temporary_beacon();
            break;
        case INTERFACE_DATA_DEL_RPI:
            ena_storage_erase_beacon();
            break;
        case INTERFACE_DATA_DEL_LAST_EXPOSURE:
            ena_storage_write_last_exposure_date(0);
            break;
        case INTERFACE_DATA_DEL_ALL:
            ena_storage_erase_all();
            break;
        }

        confirm_current = false;
        display_clear_line( 2, false);
        display_clear_line( 4, false);
        display_clear_line( 6, false);
    }
}

void interface_data_lft(void)
{
    interface_wifi_start();
}

void interface_data_rht(void)
{
    interface_settings_start();
}

void interface_data_mid(void)
{
    if (!confirm_current)
    {
        display_clear_line( 2, false);
        display_clear_line( 4, false);
        display_clear_line( 6, false);
        confirm_current = true;
    }
}

void interface_data_up(void)
{
    current_interface_data_state--;
    if (current_interface_data_state < INTERFACE_DATA_DEL_TEK)
    {
        current_interface_data_state = INTERFACE_DATA_DEL_ALL;

        current_data_index = 3;
    }
    else if (current_interface_data_state < current_data_index)
    {
        current_data_index--;
    }

    display_clear_line( 2, false);
    display_clear_line( 4, false);
    display_clear_line( 6, false);
}

void interface_data_dwn(void)
{
    current_interface_data_state++;
    if (current_interface_data_state > INTERFACE_DATA_DEL_ALL)
    {
        current_interface_data_state = INTERFACE_DATA_DEL_TEK;
        current_data_index = 0;
    }
    else if (current_interface_data_state >= (current_data_index + 3))
    {
        current_data_index++;
    }

    display_clear_line( 2, false);
    display_clear_line( 4, false);
    display_clear_line( 6, false);
}

void interface_data_display(void)
{
    display_menu_headline( interface_get_label_text(&interface_text_headline_data), true, 0);
    if (confirm_current)
    {
        display_text_line_column( interface_get_label_text(&interface_text_data_del[current_interface_data_state]), 2, 2, false);
        display_text_line_column( "?", 2, strlen(interface_get_label_text(&interface_text_data_del[current_interface_data_state])) + 2, false);

        display_set_button( interface_get_label_text(&interface_text_button_cancel), true, false);
        display_set_button( interface_get_label_text(&interface_text_button_ok), false, true);
    }
    else
    {
        display_clear_line( 5, false);
        display_clear_line( 7, false);
        for (int i = 0; i < 3; i++)
        {
            int index = i + current_data_index;
            if (index <= INTERFACE_DATA_DEL_ALL)
            {
                if (index == current_interface_data_state)
                {
                    display_data( display_gfx_arrow_right, 8, i * 2 + 2, 8, false);
                }

                display_text_line_column( interface_get_label_text(&interface_text_data_del[index]), i * 2 + 2, 2, false);
            }
        }
    }
}

void interface_data_start(void)
{
    current_interface_data_state = INTERFACE_DATA_DEL_TEK;

    interface_register_command_callback(INTERFACE_COMMAND_RST, &interface_data_rst);
    interface_register_command_callback(INTERFACE_COMMAND_SET, &interface_data_set);
    interface_register_command_callback(INTERFACE_COMMAND_LFT, &interface_data_lft);
    interface_register_command_callback(INTERFACE_COMMAND_RHT, &interface_data_rht);
    interface_register_command_callback(INTERFACE_COMMAND_MID, &interface_data_mid);
    interface_register_command_callback(INTERFACE_COMMAND_UP, &interface_data_up);
    interface_register_command_callback(INTERFACE_COMMAND_DWN, &interface_data_dwn);
    interface_set_display_function(&interface_data_display);

    ESP_LOGD(INTERFACE_LOG, "start delete data interface");
}
