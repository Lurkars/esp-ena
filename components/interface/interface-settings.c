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
    INTERFACE_SETTINGS_LOCALE = 0,
    INTERFACE_SETTINGS_TIMEZONE,
} interface_settings_state_t;

static int current_interface_settings_state;
static int current_timezone_offset = 0;

int interface_get_timezone_offset(void)
{
    return current_timezone_offset;
}

void interface_set_timezone_offset(int timezone_offset)
{
    current_timezone_offset = timezone_offset;
}

void interface_settings_set(void)
{
    interface_main_start();
}

void interface_settings_lft(void)
{
    interface_info_start();
}

void interface_settings_rht(void)
{
    interface_wifi_start();
}

void interface_settings_mid(void)
{

    current_interface_settings_state++;
    if (current_interface_settings_state > INTERFACE_SETTINGS_TIMEZONE)
    {
        current_interface_settings_state = INTERFACE_SETTINGS_LOCALE;
    }

    display_clear_line(2, false);
    display_clear_line(3, false);
    display_clear_line(4, false);
    display_clear_line(5, false);
    display_clear_line(6, false);
    display_clear_line(7, false);
}

void interface_settings_up(void)
{
    if (current_interface_settings_state == INTERFACE_SETTINGS_LOCALE)
    {
        if (interface_get_locale() == 0)
        {
            interface_set_locale(INTERFACE_NUM_LOCALE - 1);
        }
        else
        {
            interface_set_locale(interface_get_locale() - 1);
        }
    }
    else if (current_interface_settings_state == INTERFACE_SETTINGS_TIMEZONE)
    {
        current_timezone_offset++;
        if (current_timezone_offset > 12)
        {
            current_timezone_offset = -11;
        }
    }
}

void interface_settings_dwn(void)
{

    if (current_interface_settings_state == INTERFACE_SETTINGS_LOCALE)
    {
        if (interface_get_locale() + 1 == INTERFACE_NUM_LOCALE)
        {
            interface_set_locale(0);
        }
        else
        {
            interface_set_locale(interface_get_locale() + 1);
        }
    }
    else if (current_interface_settings_state == INTERFACE_SETTINGS_TIMEZONE)
    {
        current_timezone_offset--;
        if (current_timezone_offset < -11)
        {
            current_timezone_offset = 12;
        }
    }
}

void interface_settings_display(void)
{
    display_menu_headline(interface_get_label_text(&interface_text_headline_settings), true, 0);

    display_text_line_column(interface_get_label_text(&interface_text_settings_locale), 3, 1, false);

    display_text_line_column(interface_get_label_text(&interface_text_settings_timezone), 6, 1, false);

    if (current_interface_settings_state == INTERFACE_SETTINGS_LOCALE)
    {
        display_data(display_gfx_arrow_up, 8, 2, 11 * 8 + 4, false);
        display_text_line_column(
            interface_get_label_text(&interface_text_settings_locales[interface_get_locale()]), 3, 11, true);
        display_data(display_gfx_arrow_down, 8, 4, 11 * 8 + 4, false);
    }
    else
    {
        display_text_line_column(
            interface_get_label_text(&interface_text_settings_locales[interface_get_locale()]), 3, 11, false);
    }

    char timezone_char[32];
    timezone_char[0] = ' ';
    if (current_timezone_offset == 0)
    {
        timezone_char[0] = ' ';
        sprintf(&timezone_char[1], "%d", current_timezone_offset);
        timezone_char[2] = ' ';
    }
    else if (current_timezone_offset > 0)
    {
        timezone_char[0] = '+';
        sprintf(&timezone_char[1], "%d", current_timezone_offset);
        if (current_timezone_offset < 10)
        {
            timezone_char[2] = ' ';
        }
    }
    else if (current_timezone_offset < 0)
    {
        sprintf(&timezone_char[0], "%d", current_timezone_offset);
        if (current_timezone_offset > -10)
        {
            timezone_char[2] = ' ';
        }
    }

    if (current_interface_settings_state == INTERFACE_SETTINGS_TIMEZONE)
    {
        display_data(display_gfx_arrow_up, 8, 5, 7 * 8, false);
        display_chars(timezone_char, 3, 6, 6, true);
        display_data(display_gfx_arrow_down, 8, 7, 7 * 8, false);
    }
    else
    {
        display_chars(timezone_char, 3, 6, 6, false);
    }
}

void interface_settings_start(void)
{
    current_interface_settings_state = INTERFACE_SETTINGS_LOCALE;

    interface_register_command_callback(INTERFACE_COMMAND_RST, &interface_settings_mid);
    interface_register_command_callback(INTERFACE_COMMAND_SET, &interface_settings_set);
    interface_register_command_callback(INTERFACE_COMMAND_LFT, &interface_settings_lft);
    interface_register_command_callback(INTERFACE_COMMAND_RHT, &interface_settings_rht);
    interface_register_command_callback(INTERFACE_COMMAND_MID, &interface_settings_mid);
    interface_register_command_callback(INTERFACE_COMMAND_UP, &interface_settings_up);
    interface_register_command_callback(INTERFACE_COMMAND_DWN, &interface_settings_dwn);
    
    interface_set_display_function(&interface_settings_display);
}
