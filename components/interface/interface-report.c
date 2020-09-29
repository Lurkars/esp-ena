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

#include "wifi-controller.h"

#include "interface.h"

static int interface_report_tan[10] = {5, 5, 5, 5, 5, 5, 5, 5, 5, 5};
static int interface_report_tan_index = 0;

void interface_report_set(void)
{
    interface_main_start();
}

void interface_report_rst(void)
{
    // TODO: REPORT here, check tan etc.
}

void interface_report_lft(void)
{
    if (interface_report_tan_index > 0)
    {
        interface_report_tan_index--;
    }
    ESP_LOGD(INTERFACE_LOG, "tan index %d", interface_report_tan_index);
}

void interface_report_rht(void)
{
    if (interface_report_tan_index < 9)
    {
        interface_report_tan_index++;
    }
    ESP_LOGD(INTERFACE_LOG, "tan index %d", interface_report_tan_index);
}

void interface_report_up(void)
{
    if (interface_report_tan[interface_report_tan_index] > 0)
    {
        interface_report_tan[interface_report_tan_index]--;
    }
    else
    {
        interface_report_tan[interface_report_tan_index] = 9;
    }
}

void interface_report_dwn(void)
{
    if (interface_report_tan[interface_report_tan_index] < 9)
    {
        interface_report_tan[interface_report_tan_index]++;
    }
    else
    {
        interface_report_tan[interface_report_tan_index] = 0;
    }
}

void interface_report_display(void)
{
    display_menu_headline( interface_get_label_text(&interface_text_headline_tan), false, 0);

    // buttons
    display_set_button( interface_get_label_text(&interface_text_button_cancel), true, false);
    display_set_button( interface_get_label_text(&interface_text_button_ok), false, true);

    static char tan_buffer[10] = {0};

    if (interface_report_tan_index > 0)
    {
        display_data( display_gfx_arrow_left, 8, 3, 8, false);
    }
    else
    {
        display_data( display_gfx_clear, 8, 3, 8, false);
    }

    if (interface_report_tan_index < 9)
    {
        display_data( display_gfx_arrow_right, 8, 3, 112, false);
    }
    else
    {
        display_data( display_gfx_clear, 8, 3, 112, false);
    }

    for (int i = 0; i < interface_report_tan_index + 1; i++)
    {
        sprintf(&tan_buffer[i], "%d", interface_report_tan[i]);
    }

    display_clear_line( 2, false);
    display_clear_line( 4, false);

    display_text_line_column( "-", 3, 5, false);
    display_text_line_column( "-", 3, 9, false);
    display_text_line_column( "___", 3, 2, false);
    display_text_line_column( "___", 3, 6, false);
    display_text_line_column( "____", 3, 10, false);

    int offset = 2;
    for (int i = 0; i < 3; i++)
    {
        if (i < interface_report_tan_index)
        {
            display_chars( &tan_buffer[i], 1, 3, i + offset, true);
        }
    }

    if (interface_report_tan_index > 2)
    {
        offset = 3;
        for (int i = 3; i < 6; i++)
        {
            if (i < interface_report_tan_index)
            {
                display_chars( &tan_buffer[i], 1, 3, i + offset, true);
            }
        }
    }

    if (interface_report_tan_index > 5)
    {
        offset = 4;
        for (int i = 6; i < 10; i++)
        {
            if (i < interface_report_tan_index)
            {
                display_chars( &tan_buffer[i], 1, 3, i + offset, true);
            }
        }
    }

    display_data( display_gfx_arrow_up, 8, 2, interface_report_tan_index * 8 + offset * 8, false);
    display_data( display_gfx_arrow_down, 8, 4, interface_report_tan_index * 8 + offset * 8, false);

    display_chars( &tan_buffer[interface_report_tan_index], 1, 3, interface_report_tan_index + offset, false);
}

void interface_report_start(void)
{
    interface_register_command_callback(INTERFACE_COMMAND_RST, &interface_report_rst);
    interface_register_command_callback(INTERFACE_COMMAND_SET, &interface_report_set);
    interface_register_command_callback(INTERFACE_COMMAND_LFT, &interface_report_lft);
    interface_register_command_callback(INTERFACE_COMMAND_RHT, &interface_report_rht);
    interface_register_command_callback(INTERFACE_COMMAND_UP, &interface_report_up);
    interface_register_command_callback(INTERFACE_COMMAND_DWN, &interface_report_dwn);
    interface_register_command_callback(INTERFACE_COMMAND_MID, NULL);
    interface_set_display_function(&interface_report_display);

    ESP_LOGD(INTERFACE_LOG, "start report interface");
}
