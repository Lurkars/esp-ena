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

#include "ssd1306.h"
#include "ssd1306-gfx.h"

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
    ssd1306_menu_headline(SSD1306_ADDRESS, interface_get_label_text(&interface_text_headline_tan), false, 0);

    // buttons
    ssd1306_set_button(SSD1306_ADDRESS, interface_get_label_text(&interface_text_button_cancel), true, false);
    ssd1306_set_button(SSD1306_ADDRESS, interface_get_label_text(&interface_text_button_ok), false, true);

    static char tan_buffer[10] = {0};

    if (interface_report_tan_index > 0)
    {
        ssd1306_data(SSD1306_ADDRESS, ssd1306_gfx_arrow_left, 8, 3, 8, false);
    }
    else
    {
        ssd1306_data(SSD1306_ADDRESS, ssd1306_gfx_clear, 8, 3, 8, false);
    }

    if (interface_report_tan_index < 9)
    {
        ssd1306_data(SSD1306_ADDRESS, ssd1306_gfx_arrow_right, 8, 3, 112, false);
    }
    else
    {
        ssd1306_data(SSD1306_ADDRESS, ssd1306_gfx_clear, 8, 3, 112, false);
    }

    for (int i = 0; i < interface_report_tan_index + 1; i++)
    {
        sprintf(&tan_buffer[i], "%d", interface_report_tan[i]);
    }

    ssd1306_clear_line(SSD1306_ADDRESS, 2, false);
    ssd1306_clear_line(SSD1306_ADDRESS, 4, false);

    ssd1306_text_line_column(SSD1306_ADDRESS, "-", 3, 5, false);
    ssd1306_text_line_column(SSD1306_ADDRESS, "-", 3, 9, false);
    ssd1306_text_line_column(SSD1306_ADDRESS, "___", 3, 2, false);
    ssd1306_text_line_column(SSD1306_ADDRESS, "___", 3, 6, false);
    ssd1306_text_line_column(SSD1306_ADDRESS, "____", 3, 10, false);

    int offset = 2;
    for (int i = 0; i < 3; i++)
    {
        if (i < interface_report_tan_index)
        {
            ssd1306_chars(SSD1306_ADDRESS, &tan_buffer[i], 1, 3, i + offset, true);
        }
    }

    if (interface_report_tan_index > 2)
    {
        offset = 3;
        for (int i = 3; i < 6; i++)
        {
            if (i < interface_report_tan_index)
            {
                ssd1306_chars(SSD1306_ADDRESS, &tan_buffer[i], 1, 3, i + offset, true);
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
                ssd1306_chars(SSD1306_ADDRESS, &tan_buffer[i], 1, 3, i + offset, true);
            }
        }
    }

    ssd1306_data(SSD1306_ADDRESS, ssd1306_gfx_arrow_up, 8, 2, interface_report_tan_index * 8 + offset * 8, false);
    ssd1306_data(SSD1306_ADDRESS, ssd1306_gfx_arrow_down, 8, 4, interface_report_tan_index * 8 + offset * 8, false);

    ssd1306_chars(SSD1306_ADDRESS, &tan_buffer[interface_report_tan_index], 1, 3, interface_report_tan_index + offset, false);
}

void interface_report_start(void)
{
    interface_register_button_callback(INTERFACE_BUTTON_RST, &interface_report_rst);
    interface_register_button_callback(INTERFACE_BUTTON_SET, &interface_report_set);
    interface_register_button_callback(INTERFACE_BUTTON_LFT, &interface_report_lft);
    interface_register_button_callback(INTERFACE_BUTTON_RHT, &interface_report_rht);
    interface_register_button_callback(INTERFACE_BUTTON_UP, &interface_report_up);
    interface_register_button_callback(INTERFACE_BUTTON_DWN, &interface_report_dwn);
    interface_register_button_callback(INTERFACE_BUTTON_MID, NULL);
    interface_set_display_function(&interface_report_display);

    ESP_LOGD(INTERFACE_LOG, "start report interface");
}
