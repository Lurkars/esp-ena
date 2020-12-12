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

#include "ena-eke-proxy.h"

#include "interface.h"

static char current_tan[10];
static uint8_t current_cursor;
static char current_char_set[32];
static uint8_t current_char_index;
static char char_set_uppercase[32];
static char char_set_numeric[32];
static uint8_t current_max_index;

typedef enum
{
    INTERFACE_REPORT_STATUS_NONE = 0,
    INTERFACE_REPORT_STATUS_PENDING,
    INTERFACE_REPORT_STATUS_SUCCESS,
    INTERFACE_REPORT_STATUS_FAIL
} report_status_e;

static int current_report_status = INTERFACE_REPORT_STATUS_NONE;

void interface_report_display(void)
{
    if (current_report_status == INTERFACE_REPORT_STATUS_NONE)
    {
        display_menu_headline(interface_get_label_text(&interface_text_headline_tan), false, 0);
        
        // buttons
        display_set_button(interface_get_label_text(&interface_text_button_cancel), true, false);
        if (current_cursor == 9)
        {
            display_set_button(interface_get_label_text(&interface_text_button_ok), false, true);
        }

        display_clear_line(2, false);
        display_clear_line(4, false);

        display_text_line_column("-", 3, 5, false);
        display_text_line_column("-", 3, 9, false);
        display_text_line_column("___", 3, 2, false);
        display_text_line_column("___", 3, 6, false);
        display_text_line_column("____", 3, 10, false);

        int offset = 2;
        for (int i = 0; i < 3; i++)
        {
            if (i < current_cursor)
            {
                display_chars(&current_tan[i], 1, 3, i + offset, true);
            }
        }

        if (current_cursor > 2)
        {
            offset = 3;
            for (int i = 3; i < 6; i++)
            {
                if (i < current_cursor)
                {
                    display_chars(&current_tan[i], 1, 3, i + offset, true);
                }
            }
        }

        if (current_cursor > 5)
        {
            offset = 4;
            for (int i = 6; i < 10; i++)
            {
                if (i < current_cursor)
                {
                    display_chars(&current_tan[i], 1, 3, i + offset, true);
                }
            }
        }

        display_data(display_gfx_arrow_up, 8, 2, current_cursor * 8 + offset * 8, false);
        display_data(display_gfx_arrow_down, 8, 4, current_cursor * 8 + offset * 8, false);

        display_chars(&current_tan[current_cursor], 1, 3, current_cursor + offset, false);

        if (current_cursor > 0)
        {
            display_data(display_gfx_arrow_left, 8, 3, 8, false);
        }
        else
        {
            display_data(display_gfx_clear, 8, 3, 8, false);
        }

        if (current_cursor < 9)
        {
            display_data(display_gfx_arrow_right, 8, 3, 112, false);
        }
        else
        {
            display_data(display_gfx_clear, 8, 3, 112, false);
        }
    }
    else if (current_report_status == INTERFACE_REPORT_STATUS_PENDING)
    {
        display_clear();
        display_text_line_column(interface_get_label_text(&interface_text_report_pending), 4, 1, false);
        display_menu_headline(interface_get_label_text(&interface_text_headline_report), false, 0);
    }
    else if (current_report_status == INTERFACE_REPORT_STATUS_SUCCESS)
    {
        display_clear();
        display_text_line_column(interface_get_label_text(&interface_text_report_success), 3, 1, false);
        display_set_button(interface_get_label_text(&interface_text_button_ok), false, true);
        display_menu_headline(interface_get_label_text(&interface_text_headline_report), false, 0);
    }
    else if (current_report_status == INTERFACE_REPORT_STATUS_FAIL)
    {
        display_clear();
        display_text_line_column(interface_get_label_text(&interface_text_report_fail), 3, 1, false);
        display_set_button(interface_get_label_text(&interface_text_button_back), true, false);
        display_set_button(interface_get_label_text(&interface_text_button_ok), false, true);
        display_menu_headline(interface_get_label_text(&interface_text_headline_report), false, 0);
    }
}

void interface_report_set_char_set(void)
{
    char *ret;
    char cur_char = current_tan[current_cursor];

    if ((ret = strchr(char_set_uppercase, cur_char)) != NULL)
    {
        strcpy(current_char_set, char_set_uppercase);
        current_char_index = strlen(current_char_set) - strlen(ret);
    }
    else if ((ret = strchr(char_set_numeric, cur_char)) != NULL)
    {
        strcpy(current_char_set, char_set_numeric);
        current_char_index = strlen(current_char_set) - strlen(ret);
    }
}

void interface_report_set(void)
{
    if (current_report_status == INTERFACE_REPORT_STATUS_NONE)
    {
        interface_main_start();
    }
    else if (current_report_status == INTERFACE_REPORT_STATUS_FAIL)
    {
        current_report_status = INTERFACE_REPORT_STATUS_NONE;
    }
}

void interface_report_rst(void)
{
    if (current_cursor == 9 && current_report_status == INTERFACE_REPORT_STATUS_NONE)
    {
        current_report_status = INTERFACE_REPORT_STATUS_PENDING;
        interface_report_display();
        ESP_LOGI(INTERFACE_LOG, "publish tan: %s", current_tan);
        esp_err_t err = ena_eke_proxy_upload(current_tan, 0);
        if (err == ESP_OK)
        {
            current_report_status = INTERFACE_REPORT_STATUS_SUCCESS;
        }
        else
        {
            current_report_status = INTERFACE_REPORT_STATUS_FAIL;
        }
        interface_report_display();
    }
    else if (current_report_status == INTERFACE_REPORT_STATUS_SUCCESS || current_report_status == INTERFACE_REPORT_STATUS_FAIL)
    {
        interface_main_start();
    }
}

void interface_report_mid(void)
{
    if (current_report_status == INTERFACE_REPORT_STATUS_NONE)
    {
        if (current_char_set[0] == char_set_uppercase[0])
        {
            strcpy(current_char_set, char_set_numeric);
        }
        else if (current_char_set[0] == char_set_numeric[0])
        {
            strcpy(current_char_set, char_set_uppercase);
        }
        current_char_index = 0;
        current_tan[current_cursor] = current_char_set[current_char_index];
    }
}

void interface_report_lft(void)
{
    if (current_report_status == INTERFACE_REPORT_STATUS_NONE)
    {
        if (current_cursor > 0)
        {
            current_cursor--;
            interface_report_set_char_set();
        }
    }
}

void interface_report_rht(void)
{
    if (current_report_status == INTERFACE_REPORT_STATUS_NONE)
    {
        if (current_cursor < 9)
        {
            current_cursor++;

            if (current_cursor > current_max_index)
            {
                current_max_index = current_cursor;
                strcpy(current_char_set, char_set_uppercase);

                current_char_index = 0;
                current_tan[current_cursor] = current_char_set[current_char_index];
            }
            else
            {
                interface_report_set_char_set();
            }
        }
    }
}

void interface_report_up(void)
{
    if (current_report_status == INTERFACE_REPORT_STATUS_NONE)
    {
        if (current_char_index == 0)
        {
            current_char_index = strlen(current_char_set) - 1;
        }
        else
        {
            current_char_index--;
        }

        current_tan[current_cursor] = current_char_set[current_char_index];
    }
}

void interface_report_dwn(void)
{
    if (current_report_status == INTERFACE_REPORT_STATUS_NONE)
    {
        if (current_char_index == strlen(current_char_set) - 1)
        {
            current_char_index = 0;
        }
        else
        {
            current_char_index++;
        }

        current_tan[current_cursor] = current_char_set[current_char_index];
    }
}

void interface_report_start(void)
{

    display_utf8_to_ascii("ABCDEFGHIJKLMNOPQRSTUVWXYZ", char_set_uppercase);
    display_utf8_to_ascii("0123456789", char_set_numeric);

    strcpy(current_char_set, char_set_uppercase);

    current_report_status = INTERFACE_REPORT_STATUS_NONE;
    current_max_index = 0;
    current_char_index = 0;
    current_cursor = 0;
    current_tan[current_cursor] = current_char_set[current_char_index];

    interface_register_command_callback(INTERFACE_COMMAND_RST, &interface_report_rst);
    interface_register_command_callback(INTERFACE_COMMAND_SET, &interface_report_set);
    interface_register_command_callback(INTERFACE_COMMAND_MID, &interface_report_mid);
    interface_register_command_callback(INTERFACE_COMMAND_LFT, &interface_report_lft);
    interface_register_command_callback(INTERFACE_COMMAND_RHT, &interface_report_rht);
    interface_register_command_callback(INTERFACE_COMMAND_UP, &interface_report_up);
    interface_command_callback_set_trigger(INTERFACE_COMMAND_UP);
    interface_register_command_callback(INTERFACE_COMMAND_DWN, &interface_report_dwn);
    interface_command_callback_set_trigger(INTERFACE_COMMAND_DWN);
    interface_register_command_callback(INTERFACE_COMMAND_RST_LONG, &interface_report_mid);
    interface_register_command_callback(INTERFACE_COMMAND_SET_LONG, NULL);

    interface_set_display_function(&interface_report_display);
}
